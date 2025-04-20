#include "DetectionWorker.h"
#include <QThread>
#include <opencv2/imgproc.hpp>
#include <QImage>
#include <QTimer>




private:
    QTimer* m_timer = nullptr;


// ---- Constructor with parameters ----
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
    // In the constructor, create the FruitVegDetect object based on the input parameters
    detector = new FruitVegDetect(m_modelPath, m_cameraID,
        m_inputWidth, m_inputHeight,
        m_confThreshold, m_nmsThreshold);
}

DetectionWorker::~DetectionWorker()
{
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }

    if (detector) {
        delete detector;
        detector = nullptr;
    }
}

// Stop the loop
void DetectionWorker::stop()
{
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
}

// Main function of the thread
void DetectionWorker::process()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DetectionWorker::handleDetection);
    m_timer->start(30); // Trigger detection every 30ms
}


void DetectionWorker::handleDetection()
{
    cv::Mat frame;
    QMap<QString, int> detResults;

    if (detector->getDetectionFrame(frame, detResults)) {
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

        emit newFrame(qimg);
        emit newDetectionResult(detResults);
    }
}