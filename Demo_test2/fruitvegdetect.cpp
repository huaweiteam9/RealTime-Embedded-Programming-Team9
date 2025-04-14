#include "fruitvegdetect.h"
#include <iostream>

FruitVegDetect::FruitVegDetect(const std::string &modelPath, int cameraID,
                               int inputWidth, int inputHeight, float confThreshold, float nmsThreshold)
    : inputWidth(inputWidth), inputHeight(inputHeight),
    confThreshold(confThreshold), nmsThreshold(nmsThreshold)
{
    // 加载 ONNX 模型（使用绝对路径）
    net = cv::dnn::readNetFromONNX(modelPath);
    if (net.empty()) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
    } else {
        std::cout << "Successfully loaded model: " << modelPath << std::endl;
    }

    // 初始化类别名称，顺序应与模型输出相匹配
    classNames = { "apple", "cabbage", "carrot", "grape", "lemon",
                  "mango", "napa cabbage", "peach", "pepper", "potato",
                  "radish" };

    // 打开摄像头（这里使用 cameraID，一般默认为 0）
    cap.open("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", CAP_GSTREAMER);
    if (!cap.isOpened()) {
        std::cerr << "Unable to open camera with ID: " << cameraID << std::endl;
    }
    // 设置摄像头分辨率，与模型输入尺寸一致（若需要可自行调整）
    cap.set(cv::CAP_PROP_FRAME_WIDTH, inputWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, inputHeight);
}

FruitVegDetect::~FruitVegDetect()
{
    if (cap.isOpened()) {
        cap.release();
    }
}

bool FruitVegDetect::getDetectionFrame(cv::Mat &frame, QMap<QString, int> &detectionResults)
{
    cv::Mat original;
    // 捕获一帧
    cap >> original;
    if (original.empty()) {
        std::cerr << "Empty frame captured." << std::endl;
        return false;
    }
    // 克隆图像用于绘制检测结果
    cv::Mat outputFrame = original.clone();

    // ---------------- 图像预处理 ----------------
    cv::Mat blob;
    // 将 BGR 格式图像转换为模型输入格式，并归一化到 [0,1]
    cv::dnn::blobFromImage(original, blob, 1.0/255.0, cv::Size(inputWidth, inputHeight), cv::Scalar(), true, false);
    net.setInput(blob);

    // ---------------- 前向推理 ----------------
    std::vector<cv::Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames());
    if (outs.empty()) {
        std::cerr << "No outputs from network!" << std::endl;
        frame = outputFrame;
        return true; // 即使没有检测结果，也返回原图
    }
    cv::Mat outBlob = outs[0];
    if (outBlob.dims != 3) {
        std::cerr << "Output tensor dimension error: expected 3, got " << outBlob.dims << std::endl;
        frame = outputFrame;
        return true;
    }

    int numPredictions = outBlob.size[1]; // 例如 25200
    int numAttrs = outBlob.size[2];       // 例如 16

    // 将输出 reshape 为二维矩阵：每行代表一个候选框
    cv::Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());

    // ---------------- 解析检测结果 ----------------
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> classIds;
    for (int i = 0; i < detectionMat.rows; i++) {
        float cx = detectionMat.at<float>(i, 0);
        float cy = detectionMat.at<float>(i, 1);
        float w  = detectionMat.at<float>(i, 2);
        float h  = detectionMat.at<float>(i, 3);
        float objectness = detectionMat.at<float>(i, 4);

        // 获取类别分数，假设从第 5 列开始，共 11 个类别
        cv::Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
        cv::Point classIdPoint;
        double maxClassScore;
        cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint);
        float confidence = objectness * static_cast<float>(maxClassScore);
        if (confidence > confThreshold) {
            // 将候选框的中心坐标和尺寸从模型尺寸映射回原图尺寸
            int left   = static_cast<int>((cx - w / 2) * original.cols / inputWidth);
            int top    = static_cast<int>((cy - h / 2) * original.rows / inputHeight);
            int right  = static_cast<int>((cx + w / 2) * original.cols / inputWidth);
            int bottom = static_cast<int>((cy + h / 2) * original.rows / inputHeight);
            boxes.push_back(cv::Rect(cv::Point(left, top), cv::Point(right, bottom)));
            confidences.push_back(confidence);
            classIds.push_back(classIdPoint.x);
        }
    }

    // ---------------- 非最大值抑制 (NMS) ----------------
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);


    QMap<QString, int> results;
    // ---------------- 绘制检测结果 ----------------
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

        // 更新计数：转换为 QString
        if (cid >= 0 && cid < static_cast<int>(classNames.size())) {
            QString category = QString::fromStdString(classNames[cid]);
            results[category] = results.value(category, 0) + 1;
        }
    }

    frame = outputFrame;
    detectionResults = results;
    return true;
}
