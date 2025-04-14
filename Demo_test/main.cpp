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
#include <QApplication>



// 全局的 SecondPage 指针，供 handleDetections 回调使用
SecondPage* g_secondPage = nullptr;


int main(int argc, char *argv[]) {
    
    
    QApplication a(argc, argv);
    SecondPage w;
    w.show();

    return a.exec();
    //return 0;
}
