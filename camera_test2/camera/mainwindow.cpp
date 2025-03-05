#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,display(new QLabel(this))
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    // 设置 QLabel 作为中央窗口组件
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
