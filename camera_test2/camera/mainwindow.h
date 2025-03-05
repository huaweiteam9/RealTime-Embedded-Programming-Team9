#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<QMainWindow>
#include"/usr/include/opencv4/opencv2/opencv.hpp"
#include<QTimerEvent>
#include<QCoreApplication>
#include<QObject>
#include<QDebug>
#include<QTimer>
#include<QLabel>
#include<QImage>

using namespace cv;
using namespace std;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //timer
    void TimerEvent(QTimerEvent *e);
    //void startCamera (int cameraIndex = 0);
    //void stopCamera();

private slots:
    void captureFrame(); 
    void MainWindow::updateFrame();

private:
    Ui::MainWindow *ui;
    cv::VideoCapture cap;
    QTimer timer;
    QLabel *display;
    
};
#endif // MAINWINDOW_H
