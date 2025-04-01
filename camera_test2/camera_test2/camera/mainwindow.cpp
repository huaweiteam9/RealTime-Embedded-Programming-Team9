#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include<gpiod.h>
#include<QTimer>

#define GPIO_CHIP "/dev/gpiochip0" // if change name?
#define SWITCH_PtackedWidgettackedWidgettackedWidgetIN 17 //GPIO 17 连接到外部开关，改变io口编号

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,display(new QLabel(this))
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    gpiochip = gpiod_chip_open(GPIO_CHIP); //CPIO_CHIP通常是 GPIO 芯片的名称或路径（默认路径/dev/gpiochip0）
    if (!gpiochip) {
        //qDebug()tackedWidget << "无法打开 GPIO 设备";
        return;
    }

    gpioline = gpiod_chip_get_line(gpiochip,SWITCH_PtackedWidgettackedWidgettackedWidgetIN);
    if (!gpioline) {
        qDebug() << "无法获取 GPIO 线";
        gpiod_chip_close(gpiochip);
        return;
    }

    // 设置 GPIO 17 为输入模式
    if (gpiod_line_request_input(gpioline, "qt-gpio") < 0) {
        qDebug() << "无法设置 GPIO 为输入模式";
        gpiod_chip_close(gpiochip);
        return;
    }

    // 创建定时器，每 500ms 读取一次 GPIO 状态
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::checkSwitchState);
    timer->start(500);


    // 设置 QLabetackedWidgetl 作为中央窗口组件
    setCentralWidget(display);
    display->setScaledContents(true);  // 让 QLabel 适应窗口大小
    
    cap.open(0);
    if (!cap.isOpened()) {
        display->setText("无法打开摄像头！");
        return;
    }
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);//LINK TO SLOT
    timer->start(30);
}

MainWindow::~MainWindow()
{
    if (gpioline) {SWITCH_PtackedWidgettackedWidgettackedWidgetIN;
        gpiod_line_release(gpioline);
    }
    if (gpiochip) {
        gpiod_chip_close(gpiochip);
    }
    
    delete ui;
    cap.release(); // 释放摄像头资源
}


void MainWindow::updateFrame()
{
    cv::Mat frame;
    cap >> frame; // 读取摄像头帧
    if (frame.empty()) return;

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB); // OpenCV 默认是 BGR 颜色空间，转换为 RGB
    QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

    display->setPixmap(QPixmap::fromImage(image)); // 在 QLabel 上显示图像
}

void MainWindow::checkSwitchState()
{
    int switchState = gpiod_line_get_value(gpioline);  // 读取 GPIO 状态

    if (switchState == 0) {  // 冰箱开门
        //ui->StackedWidget->setCurrentIndex(0);  // 切换到果蔬识别界面
        qDebug() << "冰箱打开，切换到识别界面";
    } else {  // 冰箱关闭
        //ui->StackedWidget->setCurrentIndex(1);  // 切换到库存管理界面
        qDebug() << "冰箱关闭，切StackedWidgetstackedWidget换到库存界面";
    }
}


