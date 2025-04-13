#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unistd.h>
#include <gpiod.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "fruitcamera_english.h"
#include "secondpage.h"



int main() {
    std::thread dht_thread(dht11_thread);

    std::thread gpio(gpio_monitor_thread);
    std::thread yoloThread(camera_yolo_thread); // camera and YOLO detection thread

    dht_thread.join();
    gpio.join();
    yoloThread.join();




    return 0;
}