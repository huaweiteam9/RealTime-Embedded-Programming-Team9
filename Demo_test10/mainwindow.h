#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QCheckBox>
#include <QLabel>
#include <QThread>
#include <QMap>
#include <QString>
#include "DetectionWorker.h"  // Work thread package for fruit and vegetable inspection
#include "secondpage.h"       // second interface
#include "SensorWorker.h"
#include "GPIOSwitch.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Receive the updated image from the detection thread
    void updateImage(const QImage &img);
    // Simulates the switching page display when the state of the refrigerator door opening and closing is changed.
    void toggleFridgeState(int state);

private:
    QStackedWidget *stackedWidget;  // Stacked windows for two interfaces
    QCheckBox *fridgeSwitch;          // Virtual fridge door switch: ticked for open, unchecked for closed
    QWidget *detectionPage;           // First interface: displaying the fruit and vegetable inspection screen
    SecondPage *secondPage;           // Second interface: displays other content (e.g., tables, etc.)

    // Multi-threaded section for fruit and vegetable inspection
    QThread *workerThread;
    DetectionWorker *worker;

    // New: Interface for turning detection threads on and off
    void startDetection();
    void stopDetection();

    // New: Callback function to save test results to file
    void saveDetectionResultsToFile();

   

    // Label for displaying camera detection results in a first interface
    QLabel *detectionLabel;

    QMap<QString, int> m_latestDetectionResults;
    void updateSecondPageTable();  // Updated second interface table

private:
    // Used to keep a record of fruit (key is fruit category, value is <put in time, expiry time>)
    QMap<QString, QPair<QDateTime, QDateTime>> m_fruitTimeRecords;

private:
    SensorWorker *sensorWorker;
    QThread *sensorThread;
    QLabel *sensorLabel; // For displaying temperature and humidity data

// Add it to the private section of MainWindow:
private:
    // Original member ......
    // Added: GPIO switch section for controlling the status of the refrigerator door
    GPIOSwitch *gpioSwitch;
    QThread *gpioThread;

};

#endif // MAINWINDOW_H
