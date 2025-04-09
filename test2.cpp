//#include <opencv2/dnn.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
//#include <iostream>
//#include <vector>
//#include <string>
//
//using namespace std;
//using namespace cv;
//using namespace dnn;
//
//int main()
//{
//    // --------------------- 参数设置 ---------------------
//    // 模型文件和图片文件路径（请根据实际情况修改）
//    string modelPath = "best.onnx";  // 导出的 YOLOv5s ONNX 模型
//    string imagePath = "test7.jpg";   // 待检测图像
//
//    // 网络输入尺寸（YOLOv5s 默认输入尺寸为 640×640）
//    int inputWidth = 640;
//    int inputHeight = 640;
//
//    // 置信度阈值
//    float confThreshold = 0.25f;
//    float nmsThreshold = 0.45f;
//
//    // --------------------- 加载模型 ---------------------
//    Net net = readNetFromONNX(modelPath);
//    if (net.empty()) {
//        cerr << "加载模型失败，请检查路径: " << modelPath << endl;
//        return -1;
//    }
//    cout << "成功加载模型：" << modelPath << endl;
//
//    // --------------------- 读取并预处理图片 ---------------------
//    Mat img = imread(imagePath);
//    if (img.empty()) {
//        cerr << "无法加载图片，请检查路径: " << imagePath << endl;
//        return -1;
//    }
//
//    // 预处理：归一化、调整大小，注意 swapRB 为 true（BGR 转 RGB）
//    Mat blob;
//    blobFromImage(img, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
//    net.setInput(blob);
//
//    // --------------------- 执行前向推理 ---------------------
//    // 这里假设模型输出为一个张量，形状 [1, 25200, 16]
//    vector<Mat> outs;
//    net.forward(outs, net.getUnconnectedOutLayersNames());
//
//    cout << "网络总共得到 " << outs.size() << " 个输出" << endl;
//    for (size_t i = 0; i < outs.size(); ++i) {
//        cout << "----------------------------------" << endl;
//        cout << "输出 " << i << ":" << endl;
//        cout << "维度数： " << outs[i].dims << endl;
//        for (int j = 0; j < outs[i].dims; ++j) {
//            cout << "    第 " << j << " 维大小: " << outs[i].size[j] << endl;
//        }
//        cout << "总元素数: " << outs[i].total() << endl;
//        cout << "Channels: " << outs[i].channels() << endl;
//    }
//    cout << "----------------------------------" << endl;
//
//    // --------------------- 重塑并确认输出格式 ---------------------
//    // 假设输出为 3 维张量，形状为 [1, 25200, 16]
//    Mat outBlob = outs[0];
//    if (outBlob.dims != 3) {
//        cerr << "输出张量的维度异常，预期为 3 维，但实际为 " << outBlob.dims << " 维" << endl;
//        return -1;
//    }
//    int batch = outBlob.size[0];       // 应为1
//    int numPredictions = outBlob.size[1]; // 应为25200
//    int numAttrs = outBlob.size[2];       // 应为16
//
//    cout << "输出 Blob 的形状: [" << batch << ", " << numPredictions << ", " << numAttrs << "]" << endl;
//
//    // 将输出重塑为二维矩阵：每一行为一个候选框，每列为属性
//    Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());
//    cout << "转换后的检测矩阵尺寸: " << detectionMat.rows << " x " << detectionMat.cols << endl;
//
//    // 打印第一个候选框的数据（便于确认格式）
//    cout << "第 0 个候选框数据:" << endl;
//    for (int i = 0; i < detectionMat.cols; i++) {
//        cout << detectionMat.at<float>(0, i) << " ";
//    }
//    cout << endl;
//
//    // --------------------- 解析检测结果并绘制检测框 ---------------------
//    // 每个候选框数据格式：[cx, cy, w, h, obj_conf, score_0, score_1, ... , score_10]
//    vector<Rect> boxes;
//    vector<float> confidences;
//    vector<int> classIds;
//    for (int i = 0; i < detectionMat.rows; i++) {
//        float cx = detectionMat.at<float>(i, 0);
//        float cy = detectionMat.at<float>(i, 1);
//        float w = detectionMat.at<float>(i, 2);
//        float h = detectionMat.at<float>(i, 3);
//        float objectness = detectionMat.at<float>(i, 4);
//
//        // 取类别得分（索引 5~15），找出最大得分及对应的类别索引
//        Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
//        Point classIdPoint;
//        double maxClassScore;
//        minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
//        float confidence = objectness * float(maxClassScore);
//
//        if (confidence > confThreshold) {
//            // 根据输出假定边框数值在 inputWidth*inputHeight 尺度下（640×640），转换到原图坐标
//            int left = int((cx - w / 2) * float(img.cols) / inputWidth);
//            int top = int((cy - h / 2) * float(img.rows) / inputHeight);
//            int right = int((cx + w / 2) * float(img.cols) / inputWidth);
//            int bottom = int((cy + h / 2) * float(img.rows) / inputHeight);
//            boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
//            confidences.push_back(confidence);
//            classIds.push_back(classIdPoint.x);
//        }
//    }
//
//    // 非极大值抑制，排除重叠多余的检测框
//    vector<int> indices;
//    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
//
//    // 绘制剩余的检测框
//    for (int idx : indices) {
//        Rect box = boxes[idx];
//        rectangle(img, box, Scalar(0, 255, 0), 2);
//        // 这里仅显示类别索引及置信度，实际可用类别名替换
//        string label = format("ID:%d %.2f", classIds[idx], confidences[idx]);
//        putText(img, label, box.tl(), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
//    }
//
//    // --------------------- 显示结果图像 ---------------------
//    imshow("检测结果", img);
//    waitKey(0);
//
//    return 0;
//}



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
    // --------------------- 参数设置 ---------------------
    // 模型文件和图片文件路径（请根据实际情况修改）
    string modelPath = "best.onnx";  // 导出的 YOLOv5s ONNX 模型
    string imagePath = "test7.jpg";   // 待检测图像

    // 网络输入尺寸（YOLOv5s 默认输入尺寸为 640×640）
    int inputWidth = 640;
    int inputHeight = 640;

    // 置信度阈值及非极大值抑制阈值
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;

    // 类别名称列表，顺序对应候选框预测时第 5~15 列的 11 个类别得分
    vector<string> classNames = {
        "apple", "cabbage", "carrot", "grape", "lemon",
        "mango", "napa cabbage", "peach", "pepper", "potato",
        "radish"
    };

    // --------------------- 加载模型 ---------------------
    Net net = readNetFromONNX(modelPath);
    if (net.empty()) {
        cerr << "加载模型失败，请检查路径: " << modelPath << endl;
        return -1;
    }
    cout << "成功加载模型：" << modelPath << endl;

    // --------------------- 读取并预处理图片 ---------------------
    Mat img = imread(imagePath);
    if (img.empty()) {
        cerr << "无法加载图片，请检查路径: " << imagePath << endl;
        return -1;
    }

    // 预处理：归一化、调整大小。swapRB 设为 true（BGR 转 RGB）
    Mat blob;
    blobFromImage(img, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
    net.setInput(blob);

    // --------------------- 执行前向推理 ---------------------
    // 这里假设模型输出为一个张量，形状 [1, 25200, 16]
    vector<Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames());

    cout << "网络总共得到 " << outs.size() << " 个输出" << endl;
    for (size_t i = 0; i < outs.size(); ++i) {
        cout << "----------------------------------" << endl;
        cout << "输出 " << i << ":" << endl;
        cout << "维度数： " << outs[i].dims << endl;
        for (int j = 0; j < outs[i].dims; ++j) {
            cout << "    第 " << j << " 维大小: " << outs[i].size[j] << endl;
        }
        cout << "总元素数: " << outs[i].total() << endl;
        cout << "Channels: " << outs[i].channels() << endl;
    }
    cout << "----------------------------------" << endl;

    // --------------------- 重塑并确认输出格式 ---------------------
    // 假设输出为 3 维张量，形状为 [1, 25200, 16]
    Mat outBlob = outs[0];
    if (outBlob.dims != 3) {
        cerr << "输出张量的维度异常，预期为 3 维，但实际为 " << outBlob.dims << " 维" << endl;
        return -1;
    }
    int batch = outBlob.size[0];         // 应为1
    int numPredictions = outBlob.size[1];  // 应为25200
    int numAttrs = outBlob.size[2];        // 应为16

    cout << "输出 Blob 的形状: [" << batch << ", " << numPredictions << ", " << numAttrs << "]" << endl;

    // 将输出重塑为二维矩阵：每一行对应一个候选框，每一列为属性
    Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());
    cout << "转换后的检测矩阵尺寸: " << detectionMat.rows << " x " << detectionMat.cols << endl;

    // 打印第一个候选框的数据，便于确认格式
    cout << "第 0 个候选框数据:" << endl;
    for (int i = 0; i < detectionMat.cols; i++) {
        cout << detectionMat.at<float>(0, i) << " ";
    }
    cout << endl;

    // --------------------- 解析检测结果并绘制检测框 ---------------------
    // 数据格式：[cx, cy, w, h, obj_conf, score_0, score_1, ... , score_10]
    vector<Rect> boxes;
    vector<float> confidences;
    vector<int> classIds;
    for (int i = 0; i < detectionMat.rows; i++) {
        float cx = detectionMat.at<float>(i, 0);
        float cy = detectionMat.at<float>(i, 1);
        float w = detectionMat.at<float>(i, 2);
        float h = detectionMat.at<float>(i, 3);
        float objectness = detectionMat.at<float>(i, 4);

        // 提取类别得分（score_0 到 score_10）
        Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
        Point classIdPoint;
        double maxClassScore;
        minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
        float confidence = objectness * static_cast<float>(maxClassScore);

        if (confidence > confThreshold) {
            // 假设输出的 bbox 参数是在 640x640 尺度下，需要将其映射回原图尺寸
            int left = int((cx - w / 2) * float(img.cols) / inputWidth);
            int top = int((cy - h / 2) * float(img.rows) / inputHeight);
            int right = int((cx + w / 2) * float(img.cols) / inputWidth);
            int bottom = int((cy + h / 2) * float(img.rows) / inputHeight);
            boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
            confidences.push_back(confidence);
            // classIdPoint.x 为 0~10，对应 classNames 中的 11 个名称
            classIds.push_back(classIdPoint.x);
        }
    }

    // 非极大值抑制，排除重叠较多的检测框
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    // 绘制剩余检测框及标签（利用类别名称替换数字）
    for (int idx : indices) {
        Rect box = boxes[idx];
        rectangle(img, box, Scalar(0, 255, 0), 2);
        string label;
        int classId = classIds[idx];
        if (classId >= 0 && classId < classNames.size())
            label = format("%s %.2f", classNames[classId].c_str(), confidences[idx]);
        else
            label = format("ID:%d %.2f", classId, confidences[idx]);
        // 在边框的左上角绘制标签文本
        putText(img, label, box.tl(), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
    }

    // --------------------- 显示结果图像 ---------------------
    imshow("检测结果", img);
    waitKey(0);

    return 0;
}
