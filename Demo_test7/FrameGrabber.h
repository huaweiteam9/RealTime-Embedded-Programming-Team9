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
        // ������ͷ��ʹ���� FruitVegDetect ����ͬ�� GStreamer �ܵ���ʽ���ɸ�����Ҫ�޸ģ�
        // �˴�ʾ����ʹ��Ĭ������ͷ
        cap.open("libcamerasrc ! video/x-raw,format=RGB ! videoconvert ! appsink", cv::CAP_GSTREAMER);
        if (!cap.isOpened()) {
            qCritical("Failed to open camera with ID: %d", m_cameraID);
        }
        // ���������ķֱ���
        cap.set(cv::CAP_PROP_FRAME_WIDTH, m_width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, m_height);
    }
    ~FrameGrabber() {
        if (cap.isOpened()) {
            cap.release();
        }
    }
    void stop() { m_running = false; }

    // ���������֡
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
                // ���Ƶ�ǰ֡��������������� clone() ȷ���������ݣ�
                latestFrame = frame.clone();
            }
            // ֪ͨ����߳�����֡
            frameAvailable.wakeAll();
            // �ɸ�����Ҫ�����ɼ�Ƶ�ʣ�����ÿ��1-2ms�ȴ�һ�£�
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
