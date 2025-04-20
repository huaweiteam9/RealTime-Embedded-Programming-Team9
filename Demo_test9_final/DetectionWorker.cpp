#include "DetectionWorker.h"
#include <QThread>
#include <opencv2/imgproc.hpp>
//#include <iostream>
#include <QImage>

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
    // Clean up resources if needed
    if (detector) {
        delete detector;
        detector = nullptr;
    }
}

// Stop the loop
void DetectionWorker::stop()
{
    running = false;
}

// Main function of the thread
void DetectionWorker::process()
{
    cv::Mat frame;
    QMap<QString, int> detResults;
    while (running)
    {
        if (detector->getDetectionFrame(frame, detResults)) {
            // Convert to QImage
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
            // Emit signal to main thread to update UI
            emit newFrame(qimg);
            emit newDetectionResult(detResults);

        }
        // Sleep for 30ms each loop to avoid high CPU usage
        QThread::msleep(30);
    }
}
