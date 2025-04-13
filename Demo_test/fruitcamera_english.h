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
    // 构造函数：传入 GPIO 引脚编号
    DHT11(int pin);
    // 析构函数
    ~DHT11();

    // 读取传感器数据，返回 temperature 和 humidity，成功返回 true
    bool read(float &temperature, float &humidity);
    
private:
    int pin_number;
    gpiod_chip *chip;
    gpiod_line *line;
};

#endif // FRUITCAMERA_ENGLISH_H
