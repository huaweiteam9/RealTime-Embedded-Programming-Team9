#include <QApplication>
#include "mainwindow.h"
#include <QKeyEvent>
#include <QDebug>
#include <QMetaType>


// Define a global event filter class
class GlobalKeyFilter : public QObject {
protected:
    // Override the eventFilter method to capture keyboard events
    bool eventFilter(QObject* obj, QEvent* event) override {
        // If it's a key press event
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            // Check if the key pressed is Q (or q, which corresponds to Qt::Key_Q)
            if (keyEvent->key() == Qt::Key_Q) {
                qDebug() << "Q pressed, quitting application.";
                QApplication::quit();
                return true;  // Indicates the event has been handled and should not be propagated further
            }
        }
        // Pass on all other events
        return QObject::eventFilter(obj, event);
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Register QMap<QString, int> type to ensure it can be used with queued connections
    qRegisterMetaType<QMap<QString, int>>("QMap<QString, int>");

    // Create and install the global event filter
    GlobalKeyFilter keyFilter;
    app.installEventFilter(&keyFilter);

    MainWindow window;
    window.show();

    return app.exec();
}
