#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QMutex>
#include <QWaitCondition>

class FrameGrabber : public QObject {
    Q_OBJECT
public:
    explicit FrameGrabber(int cameraID = 0, int width = 640, int height = 640, QObject* parent = nullptr)
        : QObject(parent), m_cameraID(cameraID), m_width(width), m_height(height), m_running(true) {
        // 打开摄像头，使用与 FruitVegDetect 中相同的 GStreamer 管道格式（可根据需要修改）
        // 此处示例：使用默认摄像头
        cap.open("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", cv::CAP_GSTREAMER);
        if (!cap.isOpened()) {
            qCritical("Failed to open camera with ID: %d", m_cameraID);
        }
        // 设置期望的分辨率
        cap.set(cv::CAP_PROP_FRAME_WIDTH, m_width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, m_height);
    }
    ~FrameGrabber() {
        if (cap.isOpened()) {
            cap.release();
        }
    }
    void stop() { m_running = false; }

    // 共享的最新帧
    cv::Mat latestFrame;
    QMutex mutex;
    QWaitCondition frameAvailable;

public slots:
    void process() {
        while (m_running) {
            cv::Mat frame;
            if (!cap.read(frame) || frame.empty()) {
                qWarning("Captured empty frame.");
                continue;
            }
            {
                QMutexLocker locker(&mutex);
                // 复制当前帧到共享变量（采用 clone() 确保拷贝数据）
                latestFrame = frame.clone();
            }
            // 通知检测线程有新帧
            frameAvailable.wakeAll();
            // 可根据需要调整采集频率（例如每隔1-2ms等待一下）
            cv::waitKey(1);
        }
    }

private:
    int m_cameraID;
    int m_width;
    int m_height;
    bool m_running;
    cv::VideoCapture cap;
};

#endif // FRAMEGRABBER_H
