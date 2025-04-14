#ifndef DETECTIONWORKER_H
#define DETECTIONWORKER_H

#include <QObject>
#include <QImage>
#include <atomic>
#include <string>
#include "fruitvegdetect.h"
#include <QMap>
#include <QString>

/*
 * DetectionWorker 封装了果蔬检测工作流程，放在子线程中执行。
 */
class DetectionWorker : public QObject
{
    Q_OBJECT

public:
    // ---- 关键之处：新增一个带模型参数的构造函数 ----
    explicit DetectionWorker(const std::string &modelPath,
                             int cameraID,
                             int inputWidth,
                             int inputHeight,
                             float confThreshold,
                             float nmsThreshold,
                             QObject *parent = nullptr);
    ~DetectionWorker();

    // 停止线程循环
    void stop();

signals:
    // 每当获取到新的一帧后，发送 QImage 给主线程
    void newFrame(const QImage &frame);
    // 每当获取到检测结果（类别和数量），发送信号
    void newDetectionResult(const QMap<QString, int> &results);

public slots:
    // 在线程开始时，执行检测流程
    void process();

private:
    FruitVegDetect *detector;  // 封装摄像头与果蔬检测的类
    std::atomic<bool> running; // 用于控制循环退出的标志

    // 这里可以存储一下构造时传入的参数（如果需要在内部多处使用）
    std::string m_modelPath;
    int m_cameraID;
    int m_inputWidth;
    int m_inputHeight;
    float m_confThreshold;
    float m_nmsThreshold;
};

#endif // DETECTIONWORKER_H
