#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QCheckBox>
#include <QLabel>
#include <QThread>
#include <QMap>
#include <QString>
#include "DetectionWorker.h"  // 用于果蔬检测的工作线程封装
#include "secondpage.h"       // 第二界面
#include "SensorWorker.h"
#include "GPIOSwitch.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 接收检测线程更新的图像
    void updateImage(const QImage &img);
    // 模拟冰箱门开关状态改变时，切换页面显示
    void toggleFridgeState(int state);

private:
    QStackedWidget *stackedWidget;  // 容纳两个界面的堆叠窗口
    QCheckBox *fridgeSwitch;          // 虚拟的冰箱门开关：勾选表示打开，未勾选表示关闭
    QWidget *detectionPage;           // 第一界面：显示果蔬检测画面
    SecondPage *secondPage;           // 第二界面：显示其他内容（比如表格等）

    // 用于果蔬检测的多线程部分
    QThread *workerThread;
    DetectionWorker *worker;

    // 新增：用于开启和关闭检测线程的接口
    void startDetection();
    void stopDetection();
   

    // 用于在第一界面中显示摄像头检测结果的标签
    QLabel *detectionLabel;

    QMap<QString, int> m_latestDetectionResults;
    void updateSecondPageTable();  // 更新第二个界面表格

private:
    // 用于保存水果记录（键为水果类别，值为 <放入时间, 过期时间>）
    QMap<QString, QPair<QDateTime, QDateTime>> m_fruitTimeRecords;

private:
    SensorWorker *sensorWorker;
    QThread *sensorThread;
    QLabel *sensorLabel; // 用于显示温湿度数据

// 在 MainWindow 的 private 部分加入：
private:
    // 原有成员……
    // 新增：控制冰箱门状态的 GPIO 开关部分
    GPIOSwitch *gpioSwitch;
    QThread *gpioThread;

};

#endif // MAINWINDOW_H
