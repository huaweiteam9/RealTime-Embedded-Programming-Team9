#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include "fruitvegdetect.h"
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 定时器槽函数，更新并显示最新检测结果
    void updateFrame();

private:
    QLabel *imageLabel;          // 用于显示图像的标签控件
    QTimer *timer;               // 定时器，控制帧更新
    FruitVegDetect *detector;    // 果蔬检测对象

    // 辅助函数：将 cv::Mat 转换为 QImage
    QImage matToQImage(const cv::Mat &mat);
};

#endif // MAINWINDOW_H
