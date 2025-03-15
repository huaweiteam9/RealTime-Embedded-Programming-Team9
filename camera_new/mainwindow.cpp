#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "video_capture.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //open camera
    cap.open(0);
    //start timerevent
    startTimer(100);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::TimerEvent(QTimerEvent *e)
{
   //get info
    Mat srcImage;
    if(cap.grab())
    {
       cap.read(srcImage);//get one photo
    }

    if(srcImage.data == nullptr) return;
    //transform
    cvtColor(srcImage,srcImage,COLOR_BGR2RGB);
    QImage image(srcImage.data,srcImage.cols,srcImage.rows,srcImage.step1(),QImage::Format_RGB888);//maybe change the name(image)
    QPixmap mmp = QPixmap::fromImage(image);
    ui ->display ->setPixmap(mmp);
    //Label.setPixmap(pixmap);
    //dispaly.show
}


