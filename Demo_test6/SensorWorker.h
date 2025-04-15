#ifndef SENSORWORKER_H
#define SENSORWORKER_H

#include <QObject>

// 声明传感器数据更新信号：参数分别为温度和湿度
class SensorWorker : public QObject
{
    Q_OBJECT
public:
    // sensorPin 是传感器连接的 GPIO（这里使用 DHT11_PIN 值，例如 4，根据你实际设置调整）
    explicit SensorWorker(int sensorPin, QObject *parent = nullptr);
    ~SensorWorker();

    // 停止读取数据循环
    void stop();

signals:
    // 当读取到新的温湿度数据后，发射此信号
    void newSensorData(double temperature, double humidity);

public slots:
    // 工作线程中执行的主循环函数
    void process();

private:
    int m_sensorPin;
    bool m_running;
};

#endif // SENSORWORKER_H
