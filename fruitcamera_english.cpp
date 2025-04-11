#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace cv;
using namespace dnn;

int main()
{
    // --------------------- Parameter Settings ---------------------
    // Model file path (modify according to your situation)
    string modelPath = "best.onnx";  // Exported YOLOv5s ONNX model
    // Camera ID, default is 0
    int cameraID = 0;

    // Network input size (YOLOv5s default input is 640¡Á640)
    int inputWidth = 640;
    int inputHeight = 640;

    // Confidence threshold and non-maximum suppression (NMS) threshold
    float confThreshold = 0.7f;
    float nmsThreshold = 0.45f;

    // List of class names (corresponding to the 11 classes in the candidate boxes scores)
    vector<string> classNames = {
        "apple", "cabbage", "carrot", "grape", "lemon",
        "mango", "napa cabbage", "peach", "pepper", "potato",
        "radish"
    };

    // --------------------- Load Model ---------------------
    Net net = readNetFromONNX(modelPath);
    if (net.empty()) {
        cerr << "Failed to load model, please check the path: " << modelPath << endl;
        return -1;
    }
    cout << "Successfully loaded model: " << modelPath << endl;

    // --------------------- Open Camera ---------------------
    VideoCapture cap("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", CAP_GSTREAMER);
    if (!cap.isOpened()) {
        cerr << "Unable to open camera, please check the camera ID or connection." << endl;
        return -1;
    }

    // Set camera resolution (adjust as needed)
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 640);

    // Create window
    const string windowName = "Real-time Detection";
    namedWindow(windowName, WINDOW_NORMAL);

    // --------------------- Real-time Detection Loop ---------------------
    Mat frame;
    while (true) {
        // Read the current frame
        cap >> frame;
        if (frame.empty()) {
            cerr << "Empty frame, ending detection." << endl;
            break;
        }

        // Clone the original frame for drawing (if you want to preserve the original frame)
        Mat outputFrame = frame.clone();

        // --------------------- Image Preprocessing ---------------------
        Mat blob;
        // Resize the frame to 640x640, normalize to [0,1], and convert BGR to RGB
        blobFromImage(frame, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
        net.setInput(blob);

        // --------------------- Forward Inference ---------------------
        vector<Mat> outs;
        net.forward(outs, net.getUnconnectedOutLayersNames());

        // Assume that the model output is a 3D tensor with shape [1, 25200, 16]
        if (outs.empty()) {
            cerr << "No output result!" << endl;
            continue;
        }
        Mat outBlob = outs[0];
        if (outBlob.dims != 3) {
            cerr << "Output tensor dimension error, expected 3 dims but got " << outBlob.dims << " dims" << endl;
            continue;
        }
        int numPredictions = outBlob.size[1];  // For example, 25200
        int numAttrs = outBlob.size[2];        // For example, 16

        // Reshape the output into a 2D matrix where each row represents a candidate box
        Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());

        // --------------------- Parse Detection Results ---------------------
        vector<Rect> boxes;
        vector<float> confidences;
        vector<int> classIds;
        for (int i = 0; i < detectionMat.rows; i++) {
            float cx = detectionMat.at<float>(i, 0);
            float cy = detectionMat.at<float>(i, 1);
            float w = detectionMat.at<float>(i, 2);
            float h = detectionMat.at<float>(i, 3);
            float objectness = detectionMat.at<float>(i, 4);

            // Extract class scores (assume class scores are at indices 5 to 15, total 11 classes)
            Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
            Point classIdPoint;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
            float confidence = objectness * static_cast<float>(maxClassScore);

            if (confidence > confThreshold) {
                // Map the center coordinates and size from the 640x640 scale to the original image size
                int left = int((cx - w / 2) * float(frame.cols) / inputWidth);
                int top = int((cy - h / 2) * float(frame.rows) / inputHeight);
                int right = int((cx + w / 2) * float(frame.cols) / inputWidth);
                int bottom = int((cy + h / 2) * float(frame.rows) / inputHeight);
                boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
                confidences.push_back(confidence);
                classIds.push_back(classIdPoint.x);
            }
        }

        // --------------------- Non-maximum Suppression (NMS) ---------------------
        vector<int> indices;
        NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

        // --------------------- Draw Detection Boxes and Labels ---------------------
        for (int idx : indices) {
            Rect box = boxes[idx];
            rectangle(outputFrame, box, Scalar(0, 255, 0), 2);
            string label;
            int clsId = classIds[idx];
            if (clsId >= 0 && clsId < classNames.size())
                label = format("%s %.2f", classNames[clsId].c_str(), confidences[idx]);
            else
                label = format("ID:%d %.2f", clsId, confidences[idx]);
            putText(outputFrame, label, box.tl(), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
        }

        // --------------------- Display Results ---------------------
        imshow(windowName, outputFrame);

        // Press 'q' or ESC to exit real-time detection
        char c = (char)waitKey(1);
        if (c == 'q' || c == 27)
            break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
