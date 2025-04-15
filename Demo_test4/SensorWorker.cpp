#include "SensorWorker.h"
#include <QThread>
#include <QDebug>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <gpiod.h>

// 以下是 DHT11 类的简单包装，你可以选择直接引用你已有的代码
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
    // 读取温湿度数据，返回 true 表示成功；温度和湿度通过引用返回
    bool read(float &temperature, float &humidity) {
        uint8_t data[5] = {0};
        // 设置输出模式并拉低（18ms）
        gpiod_line_request_output(line, "dht11", 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(18));
        // 拉高40us
        gpiod_line_set_value(line, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(40));
        // 切换到输入模式
        gpiod_line_request_input(line, "dht11");
        // 等待响应（最多等待2ms）
        auto start = std::chrono::high_resolution_clock::now();
        while (gpiod_line_get_value(line) == 1) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start).count() > 2)
                return false;
        }
        // 读取 40 位数据
        for (int i = 0; i < 40; i++) {
            while (gpiod_line_get_value(line) == 0); // 等待高电平
            auto t_start = std::chrono::high_resolution_clock::now();
            while (gpiod_line_get_value(line) == 1); // 计时高电平持续时间
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t_start).count();
            data[i / 8] <<= 1;
            if (duration > 50)
                data[i / 8] |= 1;
        }
        // 校验和判断
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
    try {
        m_sensor = new DHT11(m_sensorPin);
        qDebug() << "DHT11 sensor initialized on pin" << sensorPin;
    }
    catch (const std::exception& e) {
        qCritical() << "Failed to initialize DHT11:" << e.what();
        m_sensor = nullptr;
    }
}

SensorWorker::~SensorWorker()
{
	delete m_sensor; // 确保释放资源
}

void SensorWorker::stop()
{
    m_running = false;
}

void SensorWorker::process()
{
    
    // 如果初始化失败则直接退出
    if (!m_sensor) {
        qCritical() << "DHT11 sensor not available, exiting process loop.";
        return;
    }
   
    // 为避免重复创建，若需要可以考虑将 DHT11 实例作为成员变量，这里简单采用局部变量
    while (m_running) {
        float temperature = 0, humidity = 0;
        // 稍微延迟一下，确保传感器已经稳定（例如2秒）
        QThread::sleep(2);

        // 尝试读取传感器数据
        bool success = m_sensor->read(temperature, humidity);
        if (!success) {
            qDebug() << "[DHT11] First attempt failed, retrying...";
            // 延时500毫秒后重试一次
            QThread::msleep(500);
            success = m_sensor->read(temperature, humidity);
        }
        if (success) {
            qDebug() << "[DHT11] Temperature:" << temperature << "°C, Humidity:" << humidity << "%";
            emit newSensorData(temperature, humidity);
        }
        else {
            qDebug() << "[DHT11] Second attempt failed, skipping update.";
        }

        // 设置每5秒读取一次数据
        QThread::sleep(5);
    }
}
