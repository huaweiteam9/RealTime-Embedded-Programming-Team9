#include "mainwindow.h"
#include <QHBoxLayout>
#include <QDebug>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    imageLabel(new QLabel(this)),
    timer(new QTimer(this))
{
    // 创建 FruitVegDetect 对象，假设模型文件 best.onnx 与可执行文件在同一目录下
    //detector = new FruitVegDetect("D:/qtpro/test5/test5/best.onnx", 0, 640, 640, 0.7f, 0.45f);
    detector = new FruitVegDetect("../best.onnx", 0, 640, 640, 0.7f, 0.45f);

    // 设置界面布局，将 imageLabel 填入主窗口
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);
    layout->addWidget(imageLabel);
    setCentralWidget(centralWidget);

    // 设置定时器，30ms 更新一次（大约 33fps）
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(30);
}

MainWindow::~MainWindow()
{
    if (detector) {
        delete detector;
    }
}

// 定时器槽函数，获取最新帧并更新显示
void MainWindow::updateFrame()
{
    cv::Mat detectedFrame;
    if (detector->getDetectionFrame(detectedFrame)) {
        QImage qimg = matToQImage(detectedFrame);
        imageLabel->setPixmap(QPixmap::fromImage(qimg));
    } else {
        qDebug() << "Failed to capture detection frame.";
    }
}

// 辅助函数：将 cv::Mat 转换为 QImage
QImage MainWindow::matToQImage(const cv::Mat &mat)
{
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(reinterpret_cast<const uchar*>(rgb.data), rgb.cols, rgb.rows,
                      static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    } else if (mat.type() == CV_8UC1) {
        return QImage(reinterpret_cast<const uchar*>(mat.data), mat.cols, mat.rows,
                      static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    }
    return QImage();
}
