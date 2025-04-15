#include "DetectionWorker.h"
#include "FrameGrabber.h"
#include <QMutexLocker>
#include <QElapsedTimer>

DetectionWorker::DetectionWorker(const std::string& modelPath,
    int cameraID,
    int inputWidth,
    int inputHeight,
    float confThreshold,
    float nmsThreshold,
    QObject* parent)
    : QObject(parent),
    running(true),
    m_modelPath(modelPath),
    m_cameraID(cameraID),
    m_inputWidth(inputWidth),
    m_inputHeight(inputHeight),
    m_confThreshold(confThreshold),
    m_nmsThreshold(nmsThreshold)
{
    // 初始化 FruitVegDetect 仅用于检测
    detector = new FruitVegDetect(m_modelPath, cameraID, inputWidth, inputHeight, m_confThreshold, m_nmsThreshold);
}

DetectionWorker::~DetectionWorker()
{
    if (detector) {
        delete detector;
        detector = nullptr;
    }
}

void DetectionWorker::stop()
{
    running = false;
}

void DetectionWorker::process()
{
    // 这里假定 FrameGrabber 对象已在主界面创建，并通过指针传递进来
    // 例如，可通过 setFrameGrabber(FrameGrabber* grabber) 方法传入
    if (!m_frameGrabber) {
        qCritical() << "FrameGrabber not set in DetectionWorker!";
        return;
    }

    cv::Mat frame;
    QMap<QString, int> detResults;
    while (running) {
        {
            // 锁住共享帧
            QMutexLocker locker(&m_frameGrabber->mutex);
            // 如果共享帧为空，则等待（超时可设置为 100 毫秒）
            if (m_frameGrabber->latestFrame.empty())
                m_frameGrabber->frameAvailable.wait(&m_frameGrabber->mutex, 100);
            // 复制最新的帧用于检测
            frame = m_frameGrabber->latestFrame.clone();
        }
        if (frame.empty()) {
            QThread::msleep(5);
            continue;
        }

        // 执行 YOLO 检测，检测函数内部会绘制检测结果到输出图像
        cv::Mat processedFrame;
        if (detector->detectFrame(frame, processedFrame, detResults)) {
            // 转换为 QImage
            QImage qimg;
            if (processedFrame.type() == CV_8UC3) {
                cv::Mat rgb;
                cv::cvtColor(processedFrame, rgb, cv::COLOR_BGR2RGB);
                qimg = QImage(reinterpret_cast<const uchar*>(rgb.data),
                    rgb.cols, rgb.rows,
                    static_cast<int>(rgb.step),
                    QImage::Format_RGB888).copy();
            }
            else if (processedFrame.type() == CV_8UC1) {
                qimg = QImage(reinterpret_cast<const uchar*>(processedFrame.data),
                    processedFrame.cols, processedFrame.rows,
                    static_cast<int>(processedFrame.step),
                    QImage::Format_Grayscale8).copy();
            }
            // 发出信号给 UI
            emit newFrame(qimg);
            emit newDetectionResult(detResults);
        }
        // 可适当 sleep，避免CPU占用过高
        QThread::msleep(10);
    }
}
