#include "SensorWorker.h"
#include <QThread>
#include <QDebug>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <gpiod.h>

// Here's a simple wrapper for the DHT11 class, which you can choose to directly reference your existing code
class DHT11 {
public:
    DHT11(int pin) : pin_number(pin) {
        chip = gpiod_chip_open("/dev/gpiochip0");
        if (!chip) {
            throw std::runtime_error("Failed to open GPIO chip");
        }
        line = gpiod_chip_get_line(chip, pin);
        if (!line) {
            throw std::runtime_error("Failed to get GPIO line");
        }
    }
    ~DHT11() {
        if (chip) gpiod_chip_close(chip);
    }
    // Read temperature and humidity data, return true for success; temperature and humidity are returned by reference.
    bool read(float &temperature, float &humidity) {
        uint8_t data[5] = {0};
        // Set output mode and pull low (18ms)
        gpiod_line_request_output(line, "dht11", 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(18));
        // Pull up 40us
        gpiod_line_set_value(line, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(40));
        // Switch to input mode
        gpiod_line_request_input(line, "dht11");
        // Waiting for response (up to 2ms)
        auto start = std::chrono::high_resolution_clock::now();
        while (gpiod_line_get_value(line) == 1) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start).count() > 2)
                return false;
        }
        // Reads 40-bit data
        for (int i = 0; i < 40; i++) {
            while (gpiod_line_get_value(line) == 0); // Wait for high level
            auto t_start = std::chrono::high_resolution_clock::now();
            while (gpiod_line_get_value(line) == 1); // Timing High Level Duration
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t_start).count();
            data[i / 8] <<= 1;
            if (duration > 50)
                data[i / 8] |= 1;
        }
        // checksum judgement
        if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
            return false;
        humidity = data[0] + data[1] * 0.1f;
        temperature = data[2] + data[3] * 0.1f;
        return true;
    }

private:
    int pin_number;
    gpiod_chip *chip;
    gpiod_line *line;
};

SensorWorker::SensorWorker(int sensorPin, QObject *parent)
    : QObject(parent), m_sensorPin(sensorPin), m_running(true)
{
    // You can do initialisation operations here, such as checking whether the GPIOs are active or not.
    qDebug() << "SensorWorker initialized on pin" << sensorPin;
}

SensorWorker::~SensorWorker()
{
}

void SensorWorker::stop()
{
    m_running = false;
}

void SensorWorker::process()
{
    // In order to avoid duplicate creation, you can consider the DHT11 instance as a member variable if needed, and here we simply use local variable
    while (m_running) {
        try {
            DHT11 sensor(m_sensorPin);
            float temperature = 0, humidity = 0;
            // Wait 2 seconds to ensure the sensor is ready
            std::this_thread::sleep_for(std::chrono::seconds(2));
            if (sensor.read(temperature, humidity)) {
                qDebug() << "[DHT11] Temperature:" << temperature << "°C, Humidity:" << humidity << "%";
                emit newSensorData(temperature, humidity);
            } else {
                qDebug() << "[DHT11] Failed to read data from sensor";
            }
        } catch (const std::exception &e) {
            qCritical() << "[DHT11] Exception:" << e.what();
        }
        // Set the reading interval as required, e.g. every 5 seconds.
        QThread::sleep(5);
    }
}
