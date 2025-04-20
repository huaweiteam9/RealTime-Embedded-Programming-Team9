#include "mainwindow.h"
#include "SensorWorker.h"
#include <QLabel>
#include <QThread>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>



void MainWindow::saveDetectionResultsToFile()
{
    // Open file: open file as text-only (overwrite if file exists)
    QFile file("detection_results.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open file to save detection results!";
        return;
    }

    QTextStream out(&file);
    // traverse the detection results (m_latestDetectionResults is of type QMap<QString, int>)
    QMapIterator<QString, int> it(m_latestDetectionResults);
    while (it.hasNext()) {
        it.next();
    // Keep the name and number of each category
        out << it.key() << ": " << it.value() << "\n";
    }
    file.close();
    qDebug() << "Detection results saved to detection_results.txt";
}


 // Update the detection image, scaling it proportionally to the current size of the detectionLabel.
void MainWindow::updateImage(const QImage &img)
{
    if (stackedWidget->currentIndex() == 0) {
        QPixmap pix = QPixmap::fromImage(img);
        // Get the current size of the detection tag
        QSize labelSize = detectionLabel->size();
        // Scale the image proportionally to the size of the detected label, maintaining the aspect ratio and using a smooth transformation
        QPixmap scaledPix = pix.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        detectionLabel->setPixmap(scaledPix);
    }
}


void MainWindow::updateSecondPageTable()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QMap<QString, QPair<QDateTime, QDateTime>> newFruitRecords;

    // Mapping the corresponding shelf life of each fruit and vegetable (days)
    QMap<QString, int> shelfLifeMap = {
        {"apple", 28},
        {"cabbage", 28},
        {"carrot", 28},
        {"grape", 7},
        {"lemon", 28},
        {"mango", 5},
        {"napa cabbage", 56},
        {"peach", 3},
        {"pepper", 14},
        {"potato", 7},     
        {"radish", 21}
    };

    // Traverse the test results and set the shelf life
    QMapIterator<QString, int> it(m_latestDetectionResults);
    while (it.hasNext()) {
        it.next();
        QString category = it.key();
        QString lowerCategory = category.toLower(); // Guaranteed to match lowercase key names

        if (m_fruitTimeRecords.contains(category)) {
            newFruitRecords[category] = m_fruitTimeRecords.value(category);
        }
        else {
            int shelfLifeDays = shelfLifeMap.value(lowerCategory, 1); 
            newFruitRecords[category] = qMakePair(currentTime, currentTime.addDays(shelfLifeDays));
        }
    }

    m_fruitTimeRecords = newFruitRecords;
    secondPage->clearTable();

    // Displayed in a table
    QMapIterator<QString, int> it2(m_latestDetectionResults);
    while (it2.hasNext()) {
        it2.next();
        QString category = it2.key();
        int count = it2.value();
        QPair<QDateTime, QDateTime> times = m_fruitTimeRecords.value(category);
        QString putTimeStr = times.first.toString("yyyy-MM-dd HH:mm:ss");
        QString expiryTimeStr = times.second.toString("yyyy-MM-dd HH:mm:ss");

        secondPage->addRow(category, QString::number(count), putTimeStr, expiryTimeStr);
    }
}

// Switching interface according to the switching status of the refrigerator: the detection page is displayed when the refrigerator door is open; the second page is displayed when it is closed.
void MainWindow::toggleFridgeState(int state)
{
    if (state == Qt::Checked){
        stackedWidget->setCurrentIndex(0);
    }else{
        // When the refrigerator door is closed:
        // First call the callback function to save the current detection results to a text file
        saveDetectionResultsToFile();
        stackedWidget->setCurrentIndex(1);
        // Update second page of table data when refrigerator is off
        updateSecondPageTable();
         qDebug() << "Fridge closed, updating second page table.";
    }




}


void MainWindow::startDetection()
{
    // If the detection thread already exists it is not created again.
    if (workerThread && workerThread->isRunning())
        return;

    // Creating a new detection thread and detection object
    workerThread = new QThread(this);
    
    worker = new DetectionWorker("best.onnx", 0, 640, 640, 0.7f, 0.45f);
    // Moving the detection object to the thread
    worker->moveToThread(workerThread);

    // Running the detection process at the start of the connection thread
    connect(workerThread, &QThread::started, worker, &DetectionWorker::process);
    // Update UI when detecting new frames in the thread
    connect(worker, &DetectionWorker::newFrame, this, &MainWindow::updateImage);
    // Connection of test result signals
    connect(worker, &DetectionWorker::newDetectionResult,
            this, [this](const QMap<QString, int>& results) {
                m_latestDetectionResults = results;
                qDebug() << "Updated detection results:" << m_latestDetectionResults;
            });
    // When MainWindow closes, make sure the detection thread stops and cleans up the threads
    connect(this, &MainWindow::destroyed, worker, &DetectionWorker::stop);
    connect(this, &MainWindow::destroyed, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    workerThread->start();
    qDebug() << "Detection started (camera and worker thread running)";
}

void MainWindow::stopDetection()
{
    if (workerThread) {
        if (workerThread->isRunning()) {
            // Tells the detection object to stop the loop
            worker->stop();
            // Notify the thread to exit and wait for it to finish
            workerThread->quit();
            workerThread->wait();
        }
        // Clean up detection threads and objects
        delete workerThread;  // Since we set MainWindow to be the parent, deleting the workerThread will also delete the internal subclass, but here the worker is already deleted by the finished signal.
        workerThread = nullptr;
        worker = nullptr;
        qDebug() << "Detection stopped (camera released)";
    }
}




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Setting the window to maximise (fill the screen) on startup
    this->setWindowState(Qt::WindowMaximized);

    // --- Creation of a virtual refrigerator switch ---
    // Ticked to indicate that the fridge door is open (displaying the detection screen), unchecked to indicate that the fridge door is closed (displaying the second screen)
    fridgeSwitch = new QCheckBox("Refrigerator Open", this);
    fridgeSwitch->setChecked(true);
    fridgeSwitch->hide();

    // --- Creating Stacked Windows (QStackedWidget) ---
    stackedWidget = new QStackedWidget(this);

    // First screen: Detection page (camera screen)
    detectionPage = new QWidget(this);
    QVBoxLayout *detectionLayout = new QVBoxLayout(detectionPage);
    detectionLabel = new QLabel(detectionPage);
    detectionLabel->setAlignment(Qt::AlignCenter);
    // Make detectionLabel expand automatically in the layout
    detectionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Turn off auto-stretching of content (we use manual scaling to maintain the aspect ratio)
    detectionLabel->setScaledContents(false);
    detectionLayout->addWidget(detectionLabel);
    detectionPage->setLayout(detectionLayout);

    // Second interface: e.g. a page containing a table (assuming you have implemented it)
    secondPage = new SecondPage(this);

    // Add two pages to the stacked window, with index 0 being the test page and index 1 being the second page
    stackedWidget->addWidget(detectionPage);
    stackedWidget->addWidget(secondPage);
    stackedWidget->setCurrentIndex(fridgeSwitch->isChecked() ? 0 : 1);


     //  Added: create QLabel for temperature and humidity display ---
     sensorLabel = new QLabel("", this);
     sensorLabel->setAlignment(Qt::AlignCenter);
     // A suitable font or size can be set
     sensorLabel->setStyleSheet("font-size:24px; color:blue;");

   

    // --- Creating the main layout ---
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setMargin(0);    // margin-free
    mainLayout->setSpacing(0);   // unpaced
    mainLayout->addWidget(sensorLabel);  
    mainLayout->addWidget(fridgeSwitch);
    mainLayout->addWidget(stackedWidget);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // --- Connection of refrigerator on/off state change signals for interface switching ---
    connect(fridgeSwitch, &QCheckBox::stateChanged, this, &MainWindow::toggleFridgeState);

    // --- Setting up inspection threads for camera capture and fruit and vegetable inspection ---
    workerThread = new QThread(this);
   
    worker = new DetectionWorker("best.onnx", 0, 640, 640, 0.7f, 0.45f);
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::started, worker, &DetectionWorker::process);
    // Update the interface whenever there is a new frame in the detection thread
    connect(worker, &DetectionWorker::newFrame, this, &MainWindow::updateImage);
    // Stop the detection thread and clean up resources when the window is destroyed
    connect(this, &MainWindow::destroyed, worker, &DetectionWorker::stop);
    connect(this, &MainWindow::destroyed, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(worker, &DetectionWorker::newDetectionResult,
            this, [this](const QMap<QString, int>& results) {
                // Update locally saved test data
                m_latestDetectionResults = results;
                qDebug() << "Updated detection results:" << m_latestDetectionResults;
            });

    // Starting the detection thread
    workerThread->start();




    // --- Added: Set up sensor thread for reading DHT11 temperature and humidity data ---
    int sensorPin = 26;  
    sensorWorker = new SensorWorker(sensorPin, nullptr);
    sensorThread = new QThread(this);
    sensorWorker->moveToThread(sensorThread);
    // When the thread is started, the sensor data reading main loop is executed
    connect(sensorThread, &QThread::started, sensorWorker, &SensorWorker::process);
    // Connect sensor data update signal to update sensorLabel display
    connect(sensorWorker, &SensorWorker::newSensorData, this,
            [this](double temperature, double humidity) {
                QString sensorDataStr = QString("tempurature: %1 °C, humidity: %2%")
                                            .arg(temperature, 0, 'f', 1)
                                            .arg(humidity, 0, 'f', 1);
                sensorLabel->setText(sensorDataStr);
                qDebug() << "New sensor data:" << sensorDataStr;
            });
    // Stop the sensor thread when MainWindow is destroyed
    connect(this, &MainWindow::destroyed, sensorWorker, &SensorWorker::stop);
    connect(this, &MainWindow::destroyed, sensorThread, &QThread::quit);
    connect(sensorThread, &QThread::finished, sensorWorker, &QObject::deleteLater);
    // Start the sensor thread
    sensorThread->start();

     // New GPIOSwitch object, use GPIO line number 17 (change this value if you actually wired it differently)
    // Here nullptr is passed in as the parent object for subsequent moveToThread (to avoid parent conflicts)
    gpioSwitch = new GPIOSwitch(17, nullptr);
    // Create a new dedicated thread and move the gpioSwitch to that thread
    gpioThread = new QThread(this);
    gpioSwitch->moveToThread(gpioThread);

    // Connect the gpioThread start signal to allow the gpioSwitch to start monitoring
    connect(gpioThread, &QThread::started, gpioSwitch, &GPIOSwitch::process);

    // Connect the refrigerator state change signal from GPIOSwitch to the slot of MainWindow.
    // Sets the display page of the stacked window when the switch state changes, and calls updateSecondPageTable() on closing.
    connect(gpioSwitch, &GPIOSwitch::fridgeStateChanged,
    this, [this](bool open) {
        if (open) {
            // Display the detection page when the refrigerator door is open
            startDetection();
            stackedWidget->setCurrentIndex(0);
        } else {
            // Displays second page and updates data when refrigerator door is closed
            stopDetection();
            stackedWidget->setCurrentIndex(1);
            updateSecondPageTable();
        }
        qDebug() << "Fridge state from GPIO:" << (open ? "Open" : "Closed");
    });
    // Ensure that the gpioSwitch thread is stopped when the MainWindow destroys.
    connect(this, &MainWindow::destroyed, gpioSwitch, &GPIOSwitch::stop);
    connect(this, &MainWindow::destroyed, gpioThread, &QThread::quit);
    connect(gpioThread, &QThread::finished, gpioSwitch, &QObject::deleteLater);
    // Start GPIO monitor thread
    gpioThread->start();


    
}

MainWindow::~MainWindow()
{
    if (workerThread && workerThread->isRunning()) {
        worker->stop();
        workerThread->quit();
        workerThread->wait();
    }
    if (sensorThread && sensorThread->isRunning()) {
        sensorWorker->stop();
        sensorThread->quit();
        sensorThread->wait();
    }
    if (gpioThread && gpioThread->isRunning()) {
        gpioSwitch->stop();
        gpioThread->quit();
        gpioThread->wait();
    }
}


