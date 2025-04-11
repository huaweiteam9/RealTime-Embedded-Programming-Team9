#include <gpiod.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#define GPIO_CHIP_NAME "/dev/gpiochip0"
#define DHT11_PIN 4  // GPIO4 (BCM 编号)

class DHT11 {
public:
    DHT11(int pin) : pin_number(pin) {
        chip = gpiod_chip_open(GPIO_CHIP_NAME);
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

    bool read(float &temperature, float &humidity) {
        uint8_t data[5] = {0};

        // 设置为输出并拉低
        gpiod_line_request_output(line, "dht11", 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(18));

        // 拉高 40 微秒
        gpiod_line_set_value(line, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(40));

        // 切换为输入模式
        gpiod_line_request_input(line, "dht11");

        // 等待 DHT11 响应
        auto start = std::chrono::high_resolution_clock::now();
        while (gpiod_line_get_value(line) == 1) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start).count() > 2) {
                return false;  // 超时
            }
        }

        // 读取 40 位数据
        for (int i = 0; i < 40; i++) {
            while (gpiod_line_get_value(line) == 0); // 等待信号开始

            auto t_start = std::chrono::high_resolution_clock::now();
            while (gpiod_line_get_value(line) == 1); // 计算高电平持续时间

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t_start).count();

            data[i / 8] <<= 1;
            if (duration > 50) {
                data[i / 8] |= 1;
            }
        }

        // 校验和
        if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
            return false;  // 校验失败
        }

        humidity = data[0] + data[1] * 0.1;
        temperature = data[2] + data[3] * 0.1;
        return true;
    }

private:
    int pin_number;
    gpiod_chip *chip;
    gpiod_line *line;
};

int main() {
    int i=0;
    while( i<15 )
    {
    try {
        DHT11 sensor(DHT11_PIN);
        float temperature, humidity;

        // 🌟 延迟 2 秒，确保 DHT11 初始化完成
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (sensor.read(temperature, humidity)) {
            std::cout << "Temperature: " << temperature << "°C\n";
            std::cout << "Humidity: " << humidity << "%\n";
        } else {
            std::cerr << "Failed to read data from DHT11\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    i++;
}
    return 0;
}
