#include <QApplication>
#include "mainwindow.h"
#include <QKeyEvent>
#include <QDebug>
#include <QMetaType>


// 定义一个全局事件过滤器类
class GlobalKeyFilter : public QObject {
protected:
    // 重写 eventFilter 方法，捕捉键盘事件
    bool eventFilter(QObject *obj, QEvent *event) override {
        // 如果是键盘按下事件
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            // 判断按键是否为 Q（或 q，对应 Qt::Key_Q）
            if (keyEvent->key() == Qt::Key_Q) {
                qDebug() << "Q pressed, quitting application.";
                QApplication::quit();
                return true;  // 表示该事件已处理，不再继续传递
            }
        }
        // 其他事件继续传递处理
        return QObject::eventFilter(obj, event);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 注册 QMap<QString, int> 类型，确保 queued connection 能够传递此类型
    qRegisterMetaType<QMap<QString, int>>("QMap<QString, int>");

    // 创建并安装全局事件过滤器
    GlobalKeyFilter keyFilter;
    app.installEventFilter(&keyFilter);

    MainWindow window;
    window.show();

    return app.exec();
}
