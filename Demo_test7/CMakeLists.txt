cmake_minimum_required(VERSION 3.10)
project(demo_02)

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启 Qt 的自动处理宏、UI 和资源
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 OpenCV 库
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 查找 Qt5 Widgets 模块
find_package(Qt5 COMPONENTS Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# 使用 pkg-config 查找 libgpiod（如果系统支持 pkg-config）
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
include_directories(${GPIOD_INCLUDE_DIRS})

# 包含当前源代码目录（如果需要）
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 设置需要编译的源文件和头文件
set(SOURCES
    main.cpp
    fruitvegdetect.cpp
    mainwindow.cpp
    SensorWorker.cpp
    secondpage.cpp
    DetectionWorker.cpp
    GPIOSwitch.cpp

)

set(HEADERS
    fruitvegdetect.h
    mainwindow.h
    SensorWorker.h
    secondpage.h
    DetectionWorker.h
    GPIOSwitch.h
    FrameGrabber.h
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接相关库
# - OpenCV：支持图像处理、dnn 等
# - Qt5 Widgets：提供 GUI 支持
# - GPIOD：支持 GPIO 操作
# - pthread：标准线程库（在 Linux 下需要链接）
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    Qt5::Widgets
    ${GPIOD_LIBRARIES}
    pthread
)
