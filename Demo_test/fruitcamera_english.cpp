#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unistd.h>
#include <gpiod.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <QMetaObject>
#include "fruitcamera_english.h"
#include "secondpage.h"


using namespace std;
using namespace cv;
using namespace dnn;

// 声明外部全局 SecondPage 指针（在 main.cpp 中定义）
extern SecondPage* g_secondPage;

#define GPIO_CHIP_NAME "/dev/gpiochip0"
#define CHIP_NAME "gpiochip0"
#define LINE_NUM 17
#define DHT11_PIN 4  // GPIO4 (BCM numbering)

atomic<bool> camera_on(false);
atomic<bool> shutdown_requested(false);
mutex frame_mutex;
Mat latest_frame;
atomic<bool> frame_available(false);

mutex detection_mutex;
vector<Rect> latest_boxes;
vector<int> latest_classIds;
bool detection_available = false;

vector<string> classNames = {
    "apple", "cabbage", "carrot", "grape", "lemon",
    "mango", "napa cabbage", "peach", "pepper", "potato", "radish"
};

// callbackFunction when camera turn off, output detect results 
void handleDetections(const vector<Rect>& boxes, const vector<int>& ids) {
    cout << "\n[Callback] Detection results before camera closed:\n";
    for (size_t i = 0; i < boxes.size(); ++i) {
        cout << "- Object: ";
        if (ids[i] >= 0 && ids[i] < classNames.size())
            cout << classNames[ids[i]];
        else
            cout << "ID:" << ids[i];
        cout << " at (" << boxes[i].x << "," << boxes[i].y << ","
            << boxes[i].width << "x" << boxes[i].height << ")" << endl;
        // 使用 queued connection 确保在主线程调用 SecondPage 的显示方法
        if (g_secondPage) {
            QMetaObject::invokeMethod(g_secondPage, "showPage", Qt::QueuedConnection);
        }
    }
}

// GPIO monitor threat(with callback active)
void gpio_monitor_thread() {
    gpiod_chip *chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        cerr << "cannot open GPIO: " << CHIP_NAME << endl;
        return;
    }

    gpiod_line *line = gpiod_chip_get_line(chip, LINE_NUM);
    if (!line) {
        cerr << "cannot obtain GPIO id: " << LINE_NUM << endl;
        gpiod_chip_close(chip);
        return;
    }

    if (gpiod_line_request_input_flags(line, "switch_monitor", GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP) < 0) {
        cerr << "GPIO input_mode failed" << endl;
        gpiod_chip_close(chip);
        return;
    }

    cout << "Monitoring GPIO17... (press Ctrl+C to exit)" << endl;
    int prev_value = gpiod_line_get_value(line);

    while (true) {
        int value = gpiod_line_get_value(line);
        camera_on = (value == 1);  // high value = open

        if (prev_value == 1 && value == 0) {
            // callback when status from 1 to 0
            lock_guard<mutex> lock(detection_mutex);
            if (detection_available) {
                handleDetections(latest_boxes, latest_classIds);
                detection_available = false;
            } else {
                cout << "[Callback] No detection results available.\n";
            }
        }

        prev_value = value;
        usleep(200000);  // 200ms
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
}



void camera_yolo_thread() {
    string modelPath = "best.onnx";
    int inputWidth = 640, inputHeight = 640;
    float confThreshold = 0.7f, nmsThreshold = 0.45f;

    Net net = readNetFromONNX(modelPath);
    if (net.empty()) {
        cerr << "Failed to load model: " << modelPath << endl;
        return;
    }
    cout << "Model loaded: " << modelPath << endl;

    namedWindow("YOLO Detection", WINDOW_NORMAL);

    VideoCapture cap;
    bool cameraOpened = false;

    while (true) {
        if (camera_on && !cameraOpened) {
            cap.open("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", CAP_GSTREAMER);
            if (!cap.isOpened()) {
                cerr << "Unable to open camera." << endl;
                continue;
            }
            //cap.set(CAP_PROP_FRAME_WIDTH, 1280);
            //cap.set(CAP_PROP_FRAME_HEIGHT, 960);
            cameraOpened = true;
            cout << "Camera started." << endl;
        }

        if (!camera_on && cameraOpened) {
            cap.release();
            destroyAllWindows();
            cameraOpened = false;
            cout << "Camera stopped." << endl;
            usleep(200000);
            continue;
        }

        if (!cameraOpened) {
            usleep(200000);
            continue;
        }

        Mat frame;
        cap >> frame;
        if (frame.empty()) continue;

        Mat blob;
        blobFromImage(frame, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
        net.setInput(blob);

        vector<Mat> outs;
        net.forward(outs, net.getUnconnectedOutLayersNames());

        Mat outBlob = outs[0];
        if (outBlob.dims != 3) continue;

        int numPredictions = outBlob.size[1];
        int numAttrs = outBlob.size[2];
        Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());

        vector<Rect> boxes;
        vector<float> confidences;
        vector<int> classIds;

        for (int i = 0; i < detectionMat.rows; i++) {
            float cx = detectionMat.at<float>(i, 0);
            float cy = detectionMat.at<float>(i, 1);
            float w = detectionMat.at<float>(i, 2);
            float h = detectionMat.at<float>(i, 3);
            float objectness = detectionMat.at<float>(i, 4);

            Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
            Point classIdPoint;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
            float confidence = objectness * static_cast<float>(maxClassScore);

            if (confidence > confThreshold) {
                int left = int((cx - w / 2) * float(frame.cols) / inputWidth);
                int top = int((cy - h / 2) * float(frame.rows) / inputHeight);
                int right = int((cx + w / 2) * float(frame.cols) / inputWidth);
                int bottom = int((cy + h / 2) * float(frame.rows) / inputHeight);
                boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
                confidences.push_back(confidence);
                classIds.push_back(classIdPoint.x);
            }
        }

        vector<int> indices;
        NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

        // render box + save detection result
        {
            lock_guard<mutex> lock(detection_mutex);
            latest_boxes.clear();
            latest_classIds.clear();
            for (int idx : indices) {
                latest_boxes.push_back(boxes[idx]);
                latest_classIds.push_back(classIds[idx]);
            }
            detection_available = !latest_boxes.empty();
        }

        for (int idx : indices) {
            Rect box = boxes[idx];
            rectangle(frame, box, Scalar(0, 255, 0), 2);
            string label;
            int clsId = classIds[idx];
            if (clsId >= 0 && clsId < classNames.size())
                label = format("%s %.2f", classNames[clsId].c_str(), confidences[idx]);
            else
                label = format("ID:%d %.2f", clsId, confidences[idx]);
            putText(frame, label, box.tl(), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
        }

        imshow("YOLO Detection", frame);
        char c = (char)waitKey(1);
        if (c == 'q' || c == 27) break;
    }

    if (cap.isOpened()) cap.release();
    destroyAllWindows();
}

void dht11_thread() {
    for (int i = 1; i > 0; ++i) {
        try {
            DHT11 sensor(DHT11_PIN);
            float temperature, humidity;

            std::this_thread::sleep_for(std::chrono::seconds(2));  // Ensure DHT11 is ready

            if (sensor.read(temperature, humidity)) {
                std::cout << "[DHT11] Temperature: " << temperature << "癈\n";
                std::cout << "[DHT11] Humidity: " << humidity << "%\n";
            } else {
                std::cerr << "[DHT11] Failed to read data from sensor\n";
            }
        } catch (const std::exception &e) {
            std::cerr << "[DHT11] Error: " << e.what() << "\n";
        }
    }
}

