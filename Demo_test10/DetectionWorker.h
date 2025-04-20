#ifndef DETECTIONWORKER_H
#define DETECTIONWORKER_H

#include <QObject>
#include <QImage>
#include <atomic>
#include <string>
#include "fruitvegdetect.h"
#include <QMap>
#include <QString>


class QTimer;

/*
 * DetectionWorker encapsulates the fruit and vegetable detection workflow, executed in a sub-thread.
 */
class DetectionWorker : public QObject
{
    Q_OBJECT

public:
    // ---- Key point: Add a constructor with model parameters ----
    explicit DetectionWorker(const std::string& modelPath,
        int cameraID,
        int inputWidth,
        int inputHeight,
        float confThreshold,
        float nmsThreshold,
        QObject* parent = nullptr);
    ~DetectionWorker();

    // Stop the thread loop
    void stop();

signals:
    // Emit a QImage to the main thread each time a new frame is captured
    void newFrame(const QImage& frame);
    // Emit a signal each time detection results (categories and counts) are obtained
    void newDetectionResult(const QMap<QString, int>& results);

public slots:
    // Execute detection process when the thread starts
    void process();

private slots:
    // The function executed periodically to perform detection
    void handleDetection();

private:
    FruitVegDetect* detector;  // Class encapsulating camera and fruit/vegetable detection
    QTimer* m_timer;            // Timer to trigger detection every 30ms

    // These store the constructor parameters (in case they need to be used internally)
    std::string m_modelPath;
    int m_cameraID;
    int m_inputWidth;
    int m_inputHeight;
    float m_confThreshold;
    float m_nmsThreshold;
};

#endif // DETECTIONWORKER_H
