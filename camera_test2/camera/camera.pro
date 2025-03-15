QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += link_pkgconfig
PKGCONFIG += opencv4

unix {
    # 头文件路径
    INCLUDEPATH += /usr/include/opencv4
    INCLUDEPATH += /opt/opencv4-pc/include/opencv4
    INCLUDEPATH += /opt/opencv4-pc/include/opencv4/opencv2
    INCLUDEPATH += /opt/opencv4-pc/include/
    INCLUDEPATH += /opt/opencv4-pc/include/seeta

    # OpenCV 库链接（去掉 -lopencv_world，使用单独组件）
    LIBS += -L/usr/lib/aarch64-linux-gnu \
            -lopencv_core \
            -lopencv_imgproc \
            -lopencv_highgui \
            -lopencv_videoio \
            -lopencv_imgcodecs \
            -lopencv_objdetect \
            -lopencv_dnn \
            -lopencv_features2d

    # Qt 库链接
    LIBS += -L/usr/lib/aarch64-linux-gnu \
            -lQt5Widgets \
            -lQt5Gui \
            -lQt5Core \
            -lGL -lpthread
            
    #使用GPIO
    LIBS += -lgpiod
}

# 源文件
SOURCES += \
    main.cpp \
    mainwindow.cpp

# 头文件
HEADERS += \
    mainwindow.h

# UI 界面文件
FORMS += \
    mainwindow.ui

# 部署规则
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target