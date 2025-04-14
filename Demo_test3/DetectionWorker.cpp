#include "DetectionWorker.h"
#include <QThread>
#include <opencv2/imgproc.hpp>
//#include <iostream>
#include <QImage>

// ---- 带参数的构造函数实现 ----
DetectionWorker::DetectionWorker(const std::string &modelPath,
                                 int cameraID,
                                 int inputWidth,
                                 int inputHeight,
                                 float confThreshold,
                                 float nmsThreshold,
                                 QObject *parent)
    : QObject(parent),
    running(true),
    m_modelPath(modelPath),
    m_cameraID(cameraID),
    m_inputWidth(inputWidth),
    m_inputHeight(inputHeight),
    m_confThreshold(confThreshold),
    m_nmsThreshold(nmsThreshold)
{
    // 在构造函数中，根据传入参数创建 FruitVegDetect 对象
    detector = new FruitVegDetect(m_modelPath, m_cameraID,
                                  m_inputWidth, m_inputHeight,
                                  m_confThreshold, m_nmsThreshold);
}

DetectionWorker::~DetectionWorker()
{
    // 如果有需要，可做资源清理
    if (detector) {
        delete detector;
        detector = nullptr;
    }
}

// 停止循环
void DetectionWorker::stop()
{
    running = false;
}

// 线程主函数
void DetectionWorker::process()
{
    cv::Mat frame;
    QMap<QString, int> detResults;
    while (running)
    {
        if (detector->getDetectionFrame(frame,detResults)) {
            // 转换成 QImage
            QImage qimg;
            if (frame.type() == CV_8UC3) {
                cv::Mat rgb;
                cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
                qimg = QImage(reinterpret_cast<const uchar*>(rgb.data),
                              rgb.cols, rgb.rows,
                              static_cast<int>(rgb.step),
                              QImage::Format_RGB888).copy();
            }
            else if (frame.type() == CV_8UC1) {
                qimg = QImage(reinterpret_cast<const uchar*>(frame.data),
                              frame.cols, frame.rows,
                              static_cast<int>(frame.step),
                              QImage::Format_Grayscale8).copy();
            }
            // 发射信号给主线程更新UI
            emit newFrame(qimg);
            emit newDetectionResult(detResults);

        }
        // 每次循环睡眠30ms，防止占用过高CPU
        QThread::msleep(30);
    }
}
