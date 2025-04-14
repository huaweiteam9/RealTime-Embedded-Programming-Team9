#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QApplication>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置窗口在启动时最大化（占满屏幕）
    this->setWindowState(Qt::WindowMaximized);

    // --- 创建虚拟冰箱开关 ---
    // 勾选表示冰箱门打开（显示检测页面），未勾选表示冰箱门关闭（显示第二界面）
    fridgeSwitch = new QCheckBox("Refrigerator Open", this);
    fridgeSwitch->setChecked(true);

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

    // --- 创建主布局 ---
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setMargin(0);    // 无边距
    mainLayout->setSpacing(0);   // 无间距
    mainLayout->addWidget(fridgeSwitch);
    mainLayout->addWidget(stackedWidget);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // --- 连接冰箱开关状态改变信号，实现界面切换 ---
    connect(fridgeSwitch, &QCheckBox::stateChanged, this, &MainWindow::toggleFridgeState);

    // --- 设置检测线程，用于捕获摄像头并进行果蔬检测 ---
    workerThread = new QThread(this);
    // 请确保这里传入的路径为你实际的模型文件路径
    worker = new DetectionWorker("D:/qtpro/test8/test8/best.onnx", 0, 640, 640, 0.7f, 0.45f);
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
}

MainWindow::~MainWindow()
{
    // 停止检测线程
    if (workerThread->isRunning()) {
        worker->stop();
        workerThread->quit();
        workerThread->wait();
    }
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

    // 获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();

    // 创建一个新的水果记录，用来保存本次检测结果的记录
    QMap<QString, QPair<QDateTime, QDateTime>> newFruitRecords;

    // 遍历当前检测结果 m_latestDetectionResults
    QMapIterator<QString, int> it(m_latestDetectionResults);
    while (it.hasNext()) {
        it.next();
        QString category = it.key();
        // 如果该类别已经存在于之前的记录中，则保留原来的放入和过期时间
        if (m_fruitTimeRecords.contains(category)) {
            newFruitRecords[category] = m_fruitTimeRecords.value(category);
        } else {
            // 否则，新纪录：放入时间为当前时间，过期时间为当前时间加一天
            newFruitRecords[category] = qMakePair(currentTime, currentTime.addDays(1));
        }
    }
    // 更新持久记录，可以选择将不再检测到的水果删除（这里采用更新后的 newFruitRecords）
    m_fruitTimeRecords = newFruitRecords;

    // 清空表格数据
    secondPage->clearTable();

    // 遍历当前检测结果，将水果记录填入表格
    QMapIterator<QString, int> it2(m_latestDetectionResults);
    while (it2.hasNext()) {
        it2.next();
        QString category = it2.key();
        int count = it2.value();

        // 从记录中获取原有的放入时间和过期时间
        QPair<QDateTime, QDateTime> times = m_fruitTimeRecords.value(category);
        QString putTimeStr = times.first.toString("yyyy-MM-dd HH:mm:ss");
        QString expiryTimeStr = times.second.toString("yyyy-MM-dd HH:mm:ss");

        secondPage->addRow(category, QString::number(count), putTimeStr, expiryTimeStr);
    }


    // 清空原有表格数据
    //secondPage->clearTable();
    // 遍历检测数据，将类别和数量填入表格
    //qDebug() << "Updating second page table...";
   // QMapIterator<QString, int> it(m_latestDetectionResults);


   // while (it.hasNext()) {
    //    it.next();

        // 获取当前系统时间作为放入时间
    //    QDateTime putTime = QDateTime::currentDateTime();
        // 计算过期时间，设为放入时间的一天后
    //    QDateTime expiryTime = putTime.addDays(1);

        // 格式化时间字符串，你可以根据实际需求调整时间格式
    //    QString putTimeStr = putTime.toString("yyyy-MM-dd HH:mm:ss");
    //    QString expiryTimeStr = expiryTime.toString("yyyy-MM-dd HH:mm:ss");

    //    secondPage->addRow(it.key(), QString::number(it.value()), putTimeStr, expiryTimeStr);
   // }
}

// 根据冰箱开关状态切换界面：冰箱门打开显示检测页面；关闭时显示第二页面
void MainWindow::toggleFridgeState(int state)
{
    if (state == Qt::Checked){
        stackedWidget->setCurrentIndex(0);
    }else{
        stackedWidget->setCurrentIndex(1);
        // 冰箱关闭时更新第二页表格数据
        updateSecondPageTable();
         qDebug() << "Fridge closed, updating second page table.";
    }




}
