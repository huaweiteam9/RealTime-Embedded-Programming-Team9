#ifndef FRUITVEGDETECT_H
#define FRUITVEGDETECT_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

using namespace cv;
using namespace dnn;
using namespace std;

/*
 * FruitVegDetect 类封装了基于 YOLO 模型的果蔬检测功能，
 * 包括打开摄像头、加载模型、捕获帧以及对图像进行检测、绘制检测结果等。
 */
class FruitVegDetect
{
public:
    // 构造函数：传入模型文件路径、摄像头 ID、输入尺寸、置信度阈值、NMS 阈值
    FruitVegDetect(const std::string &modelPath, int cameraID = 0,
                   int inputWidth = 640, int inputHeight = 640,
                   float confThreshold = 0.7f, float nmsThreshold = 0.45f);
    ~FruitVegDetect();

    // 获取当前检测后的帧；处理成功返回 true，并将结果存入 frame 中，否则返回 false
    bool getDetectionFrame(Mat &frame);

private:
    VideoCapture cap;     // 摄像头捕获对象
    Net net;              // DNN 网络模型
    int inputWidth;       // 模型输入宽度
    int inputHeight;      // 模型输入高度
    float confThreshold;  // 置信度阈值
    float nmsThreshold;   // 非最大值抑制阈值
    vector<string> classNames; // 类别名称列表
};

#endif // FRUITVEGDETECT_H
