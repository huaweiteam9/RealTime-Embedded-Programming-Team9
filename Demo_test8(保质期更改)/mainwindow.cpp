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
    // 打开文件：以只写文本方式打开文件（若文件存在则覆盖）
    QFile file("detection_results.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open file to save detection results!";
        return;
    }

    QTextStream out(&file);
    // 遍历检测结果（m_latestDetectionResults 为 QMap<QString, int> 类型）
    QMapIterator<QString, int> it(m_latestDetectionResults);
    while (it.hasNext()) {
        it.next();
        // 保存每个类别的名称和数量
        out << it.key() << ": " << it.value() << "\n";
    }
    file.close();
    qDebug() << "Detection results saved to detection_results.txt";
}


// 更新检测图像，根据 detectionLabel 的当前尺寸等比缩放图像
void MainWindow::updateImage(const QImage &img)
{
    if (stackedWidget->currentIndex() == 0) {
        QPixmap pix = QPixmap::fromImage(img);
        // 获取 detectionLabel 当前的尺寸
        QSize labelSize = detectionLabel->size();
        // 按照检测标签大小等比缩放图像，保持宽高比并使用平滑转换
        QPixmap scaledPix = pix.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        detectionLabel->setPixmap(scaledPix);
    }
}

void MainWindow::updateSecondPageTable()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QMap<QString, QPair<QDateTime, QDateTime>> newFruitRecords;

    // 映射每种果蔬对应的保质期（天）
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
        {"potato", 7},     // 按你要求设定
        {"radish", 21}
    };

    // 遍历检测结果并设置保质期
    QMapIterator<QString, int> it(m_latestDetectionResults);
    while (it.hasNext()) {
        it.next();
        QString category = it.key();
        QString lowerCategory = category.toLower(); // 保证匹配小写键名

        if (m_fruitTimeRecords.contains(category)) {
            newFruitRecords[category] = m_fruitTimeRecords.value(category);
        }
        else {
            int shelfLifeDays = shelfLifeMap.value(lowerCategory, 1); // 默认 1 天
            newFruitRecords[category] = qMakePair(currentTime, currentTime.addDays(shelfLifeDays));
        }
    }

    m_fruitTimeRecords = newFruitRecords;
    secondPage->clearTable();

    // 显示在表格中
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


// 根据冰箱开关状态切换界面：冰箱门打开显示检测页面；关闭时显示第二页面
void MainWindow::toggleFridgeState(int state)
{
    if (state == Qt::Checked){
        stackedWidget->setCurrentIndex(0);
    }else{
        // 冰箱门关闭时：
        // 首先调用回调函数，保存当前的检测结果到文本文件
        saveDetectionResultsToFile();
        stackedWidget->setCurrentIndex(1);
        // 冰箱关闭时更新第二页表格数据
        updateSecondPageTable();
         qDebug() << "Fridge closed, updating second page table.";
    }




}


void MainWindow::startDetection()
{
    // 如果检测线程已经存在则不重复创建
    if (workerThread && workerThread->isRunning())
        return;

    // 创建新的检测线程和检测对象
    workerThread = new QThread(this);
    // 注意这里使用你实际的模型路径和参数
    worker = new DetectionWorker("best.onnx", 0, 640, 640, 0.7f, 0.45f);
    // 将检测对象移到线程中
    worker->moveToThread(workerThread);

    // 连接线程启动时运行检测流程
    connect(workerThread, &QThread::started, worker, &DetectionWorker::process);
    // 检测线程有新帧时更新 UI
    connect(worker, &DetectionWorker::newFrame, this, &MainWindow::updateImage);
    // 连接检测结果信号
    connect(worker, &DetectionWorker::newDetectionResult,
            this, [this](const QMap<QString, int>& results) {
                m_latestDetectionResults = results;
                qDebug() << "Updated detection results:" << m_latestDetectionResults;
            });
    // 当 MainWindow 关闭时，确保检测线程停止并清理
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
            // 告诉检测对象停止循环
            worker->stop();
            // 通知线程退出并等待结束
            workerThread->quit();
            workerThread->wait();
        }
        // 清理检测线程和对象
        delete workerThread;  // 因为我们设置了 MainWindow 为 parent，这样 delete workerThread 也会删除内部子对象，但此处已通过 finished 信号删除 worker
        workerThread = nullptr;
        worker = nullptr;
        qDebug() << "Detection stopped (camera released)";
    }
}




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置窗口在启动时最大化（占满屏幕）
    this->setWindowState(Qt::WindowMaximized);

    // --- 创建虚拟冰箱开关 ---
    // 勾选表示冰箱门打开（显示检测页面），未勾选表示冰箱门关闭（显示第二界面）
    fridgeSwitch = new QCheckBox("Refrigerator Open", this);
    fridgeSwitch->setChecked(true);
    fridgeSwitch->hide();

    // --- 创建堆叠窗口 (QStackedWidget) ---
    stackedWidget = new QStackedWidget(this);

    // 第一界面：检测页面（摄像头画面）
    detectionPage = new QWidget(this);
    QVBoxLayout *detectionLayout = new QVBoxLayout(detectionPage);
    detectionLabel = new QLabel(detectionPage);
    detectionLabel->setAlignment(Qt::AlignCenter);
    // 使 detectionLabel 在布局中自动扩展
    detectionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 关闭自动拉伸内容（我们采用手动缩放以保持宽高比）
    detectionLabel->setScaledContents(false);
    detectionLayout->addWidget(detectionLabel);
    detectionPage->setLayout(detectionLayout);

    // 第二界面：例如一个包含表格的页面（假设你已经实现）
    secondPage = new SecondPage(this);

    // 将两个页面添加到堆叠窗口中，索引 0 为检测页面，索引 1 为第二页面
    stackedWidget->addWidget(detectionPage);
    stackedWidget->addWidget(secondPage);
    stackedWidget->setCurrentIndex(fridgeSwitch->isChecked() ? 0 : 1);


     //  新增：创建温湿度显示的 QLabel ---
     sensorLabel = new QLabel("", this);
     sensorLabel->setAlignment(Qt::AlignCenter);
     // 可设置一个合适的字体或者大小
     sensorLabel->setStyleSheet("font-size:24px; color:blue;");

   

    // --- 创建主布局 ---
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setMargin(0);    // 无边距
    mainLayout->setSpacing(0);   // 无间距
    mainLayout->addWidget(sensorLabel);  
    mainLayout->addWidget(fridgeSwitch);
    mainLayout->addWidget(stackedWidget);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // --- 连接冰箱开关状态改变信号，实现界面切换 ---
    connect(fridgeSwitch, &QCheckBox::stateChanged, this, &MainWindow::toggleFridgeState);

    // --- 设置检测线程，用于捕获摄像头并进行果蔬检测 ---
    workerThread = new QThread(this);
    // 请确保这里传入的路径为你实际的模型文件路径
    worker = new DetectionWorker("best.onnx", 0, 640, 640, 0.7f, 0.45f);
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::started, worker, &DetectionWorker::process);
    // 每当检测线程有新帧时，更新界面
    connect(worker, &DetectionWorker::newFrame, this, &MainWindow::updateImage);
    // 在窗口销毁时，停止检测线程和清理资源
    connect(this, &MainWindow::destroyed, worker, &DetectionWorker::stop);
    connect(this, &MainWindow::destroyed, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(worker, &DetectionWorker::newDetectionResult,
            this, [this](const QMap<QString, int>& results) {
                // 更新本地保存的检测数据
                m_latestDetectionResults = results;
                qDebug() << "Updated detection results:" << m_latestDetectionResults;
            });

    // 启动检测线程
    workerThread->start();




    // --- 新增：设置传感器线程，用于读取 DHT11 温湿度数据 ---
    int sensorPin = 4;  // 根据你实际的接线选择正确的引脚号
    sensorWorker = new SensorWorker(sensorPin, nullptr);
    sensorThread = new QThread(this);
    sensorWorker->moveToThread(sensorThread);
    // 当线程启动时，执行传感器数据读取主循环
    connect(sensorThread, &QThread::started, sensorWorker, &SensorWorker::process);
    // 连接传感器数据更新信号，更新 sensorLabel 显示
    connect(sensorWorker, &SensorWorker::newSensorData, this,
            [this](double temperature, double humidity) {
                QString sensorDataStr = QString("tempurature: %1 °C, humidity: %2%")
                                            .arg(temperature, 0, 'f', 1)
                                            .arg(humidity, 0, 'f', 1);
                sensorLabel->setText(sensorDataStr);
                qDebug() << "New sensor data:" << sensorDataStr;
            });
    // 当 MainWindow 销毁时，停止传感器线程
    connect(this, &MainWindow::destroyed, sensorWorker, &SensorWorker::stop);
    connect(this, &MainWindow::destroyed, sensorThread, &QThread::quit);
    connect(sensorThread, &QThread::finished, sensorWorker, &QObject::deleteLater);
    // 启动传感器线程
    sensorThread->start();

     // 新建 GPIOSwitch 对象，使用 GPIO 线号 17（如果你实际接线不同，请修改此值）
    // 这里传入 nullptr 作为父对象，以便后续 moveToThread（避免父对象冲突）
    gpioSwitch = new GPIOSwitch(17, nullptr);
    // 新建专用线程，并将 gpioSwitch 移动到该线程中
    gpioThread = new QThread(this);
    gpioSwitch->moveToThread(gpioThread);

    // 连接 gpioThread 启动信号，让 gpioSwitch 开始监控
    connect(gpioThread, &QThread::started, gpioSwitch, &GPIOSwitch::process);

    // 连接 GPIOSwitch 发出的冰箱状态改变信号到 MainWindow 的槽
    // 当开关状态改变时，设置堆叠窗口的显示页面，并在关门时调用 updateSecondPageTable()
    connect(gpioSwitch, &GPIOSwitch::fridgeStateChanged,
    this, [this](bool open) {
        if (open) {
            // 当冰箱门打开时显示检测页面
            startDetection();
            stackedWidget->setCurrentIndex(0);
        } else {
            // 当冰箱门关闭时显示第二页面并更新数据
            stopDetection();
            stackedWidget->setCurrentIndex(1);
            updateSecondPageTable();
        }
        qDebug() << "Fridge state from GPIO:" << (open ? "Open" : "Closed");
    });
    // 在 MainWindow 销毁时确保停止 gpioSwitch 线程
    connect(this, &MainWindow::destroyed, gpioSwitch, &GPIOSwitch::stop);
    connect(this, &MainWindow::destroyed, gpioThread, &QThread::quit);
    connect(gpioThread, &QThread::finished, gpioSwitch, &QObject::deleteLater);
    // 启动 GPIO 监控线程
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


