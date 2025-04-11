#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <gpiod.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;
using namespace dnn;

#define CHIP_NAME "gpiochip0"
#define LINE_NUM 17  // BCM GPIO 17

atomic<bool> camera_on(false);  // 共享变量，用于控制摄像头线程

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
    while (true) {
        int value = gpiod_line_get_value(line);
        cout << gpiod_line_get_value(line) << endl;
        camera_on = (value == 1);  // 低电平表示按下（开）
        usleep(200000);  // 200 ms
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
}

void camera_yolo_thread() {
    string modelPath = "best.onnx";
    int inputWidth = 640, inputHeight = 640;
    float confThreshold = 0.7f, nmsThreshold = 0.45f;

    vector<string> classNames = {
        "apple", "cabbage", "carrot", "grape", "lemon",
        "mango", "napa cabbage", "peach", "pepper", "potato", "radish"
    };

    Net net = readNetFromONNX(modelPath);
    if (net.empty()) {
        cerr << "Failed to load model: " << modelPath << endl;
        return;
    }
    cout << "Model loaded: " << modelPath << endl;

        VideoCapture cap("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", CAP_GSTREAMER);

        namedWindow("YOLO Detection", WINDOW_NORMAL);
        // int wid_name = 0;
        
    while (true) {
        if (!camera_on) {
            usleep(1000);  // 等待开启
            
            //cap.release();
            destroyAllWindows();
            continue;
        }
        

        if (!cap.isOpened()) {
            cerr << "Unable to open camera." << endl;
            return;
        }
        // wid_name++;
        // VideoCapture cap("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", CAP_GSTREAMER);
        cap.set(CAP_PROP_FRAME_WIDTH, 640);
        cap.set(CAP_PROP_FRAME_HEIGHT, 640);
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

    cap.release();
    destroyAllWindows();
}

int main() {
    thread gpioThread(gpio_monitor_thread);
    thread yoloThread(camera_yolo_thread);

    gpioThread.join();
    yoloThread.join();

    return 0;
}
