#include <QApplication>
#include "mainwindow.h"
#include "secondpage.h"

// Application entry point
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    SecondPage secondpage;
    secondpage.show();
    return app.exec();
}
