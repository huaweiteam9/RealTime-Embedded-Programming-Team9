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
    // 模型文件路径（请根据实际情况修改）
    string modelPath = "best.onnx";  // 导出的 YOLOv5s ONNX 模型
    // 摄像头ID，默认为0
    int cameraID = 0;

    // 网络输入尺寸（YOLOv5s 默认输入为 640×640）
    int inputWidth = 640;
    int inputHeight = 640;

    // 置信度阈值及非极大值抑制阈值
    float confThreshold = 0.7f;
    float nmsThreshold = 0.45f;

    // 类别名称列表（对应输出候选框中得分的 11 个类别）
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

    // --------------------- 打开摄像头 ---------------------
    VideoCapture cap(cameraID);
    if (!cap.isOpened()) {
        cerr << "无法打开摄像头，请检查摄像头设备ID或连接状态。" << endl;
        return -1;
    }

    // 设置摄像头分辨率（可根据需要设置）
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    // 创建窗口
    const string windowName = "实时检测";
    namedWindow(windowName, WINDOW_NORMAL);

    // --------------------- 实时检测循环 ---------------------
    Mat frame;
    while (true) {
        // 读取当前帧
        cap >> frame;
        if (frame.empty()) {
            cerr << "空帧，结束检测。" << endl;
            break;
        }

        // 复制原图用于绘制（如果需要保留原始帧）
        Mat outputFrame = frame.clone();

        // --------------------- 图像预处理 ---------------------
        Mat blob;
        // 将帧调整为 640x640，归一化至 [0,1]，BGR -> RGB
        blobFromImage(frame, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
        net.setInput(blob);

        // --------------------- 前向推理 ---------------------
        vector<Mat> outs;
        net.forward(outs, net.getUnconnectedOutLayersNames());

        // 假定模型输出为 3 维张量，形状 [1, 25200, 16]
        if (outs.empty()) {
            cerr << "无输出结果！" << endl;
            continue;
        }
        Mat outBlob = outs[0];
        if (outBlob.dims != 3) {
            cerr << "输出张量维度异常，预期为 3 维，但实际为 " << outBlob.dims << " 维" << endl;
            continue;
        }
        int numPredictions = outBlob.size[1];  // 例如 25200
        int numAttrs = outBlob.size[2];        // 例如 16

        // 将输出重塑为二维矩阵，每行代表一个候选框
        Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());

        // --------------------- 解析检测结果 ---------------------
        vector<Rect> boxes;
        vector<float> confidences;
        vector<int> classIds;
        for (int i = 0; i < detectionMat.rows; i++) {
            float cx = detectionMat.at<float>(i, 0);
            float cy = detectionMat.at<float>(i, 1);
            float w = detectionMat.at<float>(i, 2);
            float h = detectionMat.at<float>(i, 3);
            float objectness = detectionMat.at<float>(i, 4);

            // 提取类别分数，假设类别得分位于索引 5 到 15（共11个类别）
            Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
            Point classIdPoint;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
            float confidence = objectness * static_cast<float>(maxClassScore);

            if (confidence > confThreshold) {
                // 将中心坐标和宽高从 640x640 尺度映射到原图尺寸
                int left = int((cx - w / 2) * float(frame.cols) / inputWidth);
                int top = int((cy - h / 2) * float(frame.rows) / inputHeight);
                int right = int((cx + w / 2) * float(frame.cols) / inputWidth);
                int bottom = int((cy + h / 2) * float(frame.rows) / inputHeight);
                boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
                confidences.push_back(confidence);
                classIds.push_back(classIdPoint.x);
            }
        }

        // --------------------- 非极大值抑制 ---------------------
        vector<int> indices;
        NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

        // --------------------- 绘制检测框和标签 ---------------------
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

        // --------------------- 显示结果 ---------------------
        imshow(windowName, outputFrame);

        // 按 'q' 键退出实时检测
        char c = (char)waitKey(1);
        if (c == 'q' || c == 27)  // 'q' 或 ESC 键退出
            break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
