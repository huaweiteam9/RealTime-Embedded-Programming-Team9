#include "fruitvegdetect.h"
#include <iostream>

FruitVegDetect::FruitVegDetect(const std::string& modelPath, int cameraID,
    int inputWidth, int inputHeight, float confThreshold, float nmsThreshold)
    : inputWidth(inputWidth), inputHeight(inputHeight),
    confThreshold(confThreshold), nmsThreshold(nmsThreshold)
{
    // Load ONNX model (use absolute path)
    net = cv::dnn::readNetFromONNX(modelPath);
    if (net.empty()) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
    }
    else {
        std::cout << "Successfully loaded model: " << modelPath << std::endl;
    }

    // Initialize class names in the order matching the model output
    classNames = { "apple", "cabbage", "carrot", "grape", "lemon",
                  "mango", "napa cabbage", "peach", "pepper", "potato",
                  "radish" };

    // Open the camera (use cameraID, usually 0 by default)
    cap.open("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", CAP_GSTREAMER);
    if (!cap.isOpened()) {
        std::cerr << "Unable to open camera with ID: " << cameraID << std::endl;
    }
    // Set camera resolution to match model input size (adjust if necessary)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, inputWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, inputHeight);
}

FruitVegDetect::~FruitVegDetect()
{
    if (cap.isOpened()) {
        cap.release();
    }
}

bool FruitVegDetect::getDetectionFrame(cv::Mat& frame, QMap<QString, int>& detectionResults)
{
    cv::Mat original;
    // Capture a frame
    cap >> original;
    if (original.empty()) {
        std::cerr << "Empty frame captured." << std::endl;
        return false;
    }
    // Clone the image for drawing detection results
    cv::Mat outputFrame = original.clone();

    // ---------------- Image Preprocessing ----------------
    cv::Mat blob;
    // Convert BGR image to input format and normalize to [0,1]
    cv::dnn::blobFromImage(original, blob, 1.0 / 255.0, cv::Size(inputWidth, inputHeight), cv::Scalar(), true, false);
    net.setInput(blob);

    // ---------------- Forward Inference ----------------
    std::vector<cv::Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames());
    if (outs.empty()) {
        std::cerr << "No outputs from network!" << std::endl;
        frame = outputFrame;
        return true; // Return original frame even if no detections
    }
    cv::Mat outBlob = outs[0];
    if (outBlob.dims != 3) {
        std::cerr << "Output tensor dimension error: expected 3, got " << outBlob.dims << std::endl;
        frame = outputFrame;
        return true;
    }

    int numPredictions = outBlob.size[1]; // e.g., 25200
    int numAttrs = outBlob.size[2];       // e.g., 16

    // Reshape output to 2D matrix: each row is a candidate box
    cv::Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());

    // ---------------- Parse Detection Results ----------------
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> classIds;
    for (int i = 0; i < detectionMat.rows; i++) {
        float cx = detectionMat.at<float>(i, 0);
        float cy = detectionMat.at<float>(i, 1);
        float w = detectionMat.at<float>(i, 2);
        float h = detectionMat.at<float>(i, 3);
        float objectness = detectionMat.at<float>(i, 4);

        // Get class scores, assuming 11 classes starting from column 5
        cv::Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
        cv::Point classIdPoint;
        double maxClassScore;
        cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint);
        float confidence = objectness * static_cast<float>(maxClassScore);
        if (confidence > confThreshold) {
            // Map box center and size from model dimensions back to original image size
            int left = static_cast<int>((cx - w / 2) * original.cols / inputWidth);
            int top = static_cast<int>((cy - h / 2) * original.rows / inputHeight);
            int right = static_cast<int>((cx + w / 2) * original.cols / inputWidth);
            int bottom = static_cast<int>((cy + h / 2) * original.rows / inputHeight);
            boxes.push_back(cv::Rect(cv::Point(left, top), cv::Point(right, bottom)));
            confidences.push_back(confidence);
            classIds.push_back(classIdPoint.x);
        }
    }

    // ---------------- Non-Maximum Suppression (NMS) ----------------
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);


    QMap<QString, int> results;
    // ---------------- Draw Detection Results ----------------
    for (int idx : indices) {
        cv::Rect box = boxes[idx];
        cv::rectangle(outputFrame, box, cv::Scalar(0, 255, 0), 2);
        std::string label;
        int cid = classIds[idx];
        if (cid >= 0 && cid < static_cast<int>(classNames.size()))
            label = cv::format("%s %.2f", classNames[cid].c_str(), confidences[idx]);
        else
            label = cv::format("ID:%d %.2f", cid, confidences[idx]);
        cv::putText(outputFrame, label, box.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);

        // Update count: convert to QString
        if (cid >= 0 && cid < static_cast<int>(classNames.size())) {
            QString category = QString::fromStdString(classNames[cid]);
            results[category] = results.value(category, 0) + 1;
        }
    }

    frame = outputFrame;
    detectionResults = results;
    return true;
}

