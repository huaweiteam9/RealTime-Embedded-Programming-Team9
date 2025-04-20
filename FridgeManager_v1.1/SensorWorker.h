#ifndef SENSORWORKER_H
#define SENSORWORKER_H

#include <QObject>

// Declare the sensor data update signal: parameters are temperature and humidity, respectively.
class SensorWorker : public QObject
{
    Q_OBJECT
public:
    // sensorPin is the GPIO to which the sensor is connected (here we use the DHT11_PIN value, e.g. 4, adjusted according to your actual settings)
    explicit SensorWorker(int sensorPin, QObject *parent = nullptr);
    ~SensorWorker();

    // Stop the read data cycle
    void stop();

signals:
    // Emits this signal when new temperature and humidity data is read
    void newSensorData(double temperature, double humidity);

public slots:
    // The main loop function executed in the worker thread
    void process();

private:
    int m_sensorPin;
    bool m_running;
};

#endif // SENSORWORKER_H
