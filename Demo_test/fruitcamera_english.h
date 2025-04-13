#ifndef FRUITCAMERA_ENGLISH_H
#define FRUITCAMERA_ENGLISH_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <unistd.h>
#include <gpiod.h>

// GPIO及DHT11相关的宏定义
#define GPIO_CHIP_NAME "/dev/gpiochip0"
#define CHIP_NAME "gpiochip0"
#define LINE_NUM 17
#define DHT11_PIN 4  // 使用 BCM 编号

// 全局变量声明
extern std::atomic<bool> camera_on;
extern std::atomic<bool> shutdown_requested;
extern std::mutex frame_mutex;
extern cv::Mat latest_frame;
extern std::atomic<bool> frame_available;

extern std::mutex detection_mutex;
extern std::vector<cv::Rect> latest_boxes;
extern std::vector<int> latest_classIds;
extern bool detection_available;

extern std::vector<std::string> classNames;

// 回调函数声明，用于摄像头关闭时输出检测结果
void handleDetections(const std::vector<cv::Rect>& boxes, const std::vector<int>& ids);

// GPIO监控线程函数声明
void gpio_monitor_thread();

// 摄像头检测与YOLO识别线程函数声明
void camera_yolo_thread();

// DHT11温湿度传感器线程函数声明
void dht11_thread();

// DHT11类声明
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

        // set to output mode and pull low 
        gpiod_line_request_output(line, "dht11", 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(18));

        // pull high for 40 us
        gpiod_line_set_value(line, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(40));

        // Switch to input mode
        gpiod_line_request_input(line, "dht11");

        // Wait for DHT11 response
        auto start = std::chrono::high_resolution_clock::now();
        while (gpiod_line_get_value(line) == 1) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start).count() > 2) {
                return false;  // Timeout
            }
        }

        // read 40 bits of data
        for (int i = 0; i < 40; i++) {
            while (gpiod_line_get_value(line) == 0); // wait for signal to go high

            auto t_start = std::chrono::high_resolution_clock::now();
            while (gpiod_line_get_value(line) == 1); // measure high signal duration

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t_start).count();

            data[i / 8] <<= 1;
            if (duration > 50) {
                data[i / 8] |= 1;
            }
        }

        // checksum validation
        if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
            return false;  // checksum failed
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

#endif // FRUITCAMERA_ENGLISH_H
