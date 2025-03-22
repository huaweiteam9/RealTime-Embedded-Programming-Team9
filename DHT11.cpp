#include <gpiod.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#define GPIO_CHIP_NAME "/dev/gpiochip0"
#define DHT11_PIN 4  // GPIO4 (BCM ���)

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

    bool read(float& temperature, float& humidity) {
        uint8_t data[5] = { 0 };

        // ����Ϊ���������
        gpiod_line_request_output(line, "dht11", 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(18));

        // ���� 40 ΢��
        gpiod_line_set_value(line, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(40));

        // �л�Ϊ����ģʽ
        gpiod_line_request_input(line, "dht11");

        // �ȴ� DHT11 ��Ӧ
        auto start = std::chrono::high_resolution_clock::now();
        while (gpiod_line_get_value(line) == 1) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start).count() > 2) {
                return false;  // ��ʱ
            }
        }

        // ��ȡ 40 λ����
        for (int i = 0; i < 40; i++) {
            while (gpiod_line_get_value(line) == 0); // �ȴ��źſ�ʼ

            auto t_start = std::chrono::high_resolution_clock::now();
            while (gpiod_line_get_value(line) == 1); // ����ߵ�ƽ����ʱ��

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t_start).count();

            data[i / 8] <<= 1;
            if (duration > 50) {
                data[i / 8] |= 1;
            }
        }

        // У���
        if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
            return false;  // У��ʧ��
        }

        humidity = data[0] + data[1] * 0.1;
        temperature = data[2] + data[3] * 0.1;
        return true;
    }

private:
    int pin_number;
    gpiod_chip* chip;
    gpiod_line* line;
};

int main() {
    try {
        DHT11 sensor(DHT11_PIN);
        float temperature, humidity;

        //  �ӳ� 2 �룬ȷ�� DHT11 ��ʼ�����
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (sensor.read(temperature, humidity)) {
            std::cout << "Temperature: " << temperature << "��C\n";
            std::cout << "Humidity: " << humidity << "%\n";
        }
        else {
            std::cerr << "Failed to read data from DHT11\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}