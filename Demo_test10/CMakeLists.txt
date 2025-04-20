cmake_minimum_required(VERSION 3.10)
project(FridgeManager)

# Using C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic processing of macros, UI and resources
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Finding OpenCV Libraries
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Finding the Qt5 Widgets Module
find_package(Qt5 COMPONENTS Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# Use pkg-config to find libgpiod (if your system supports pkg-config)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
include_directories(${GPIOD_INCLUDE_DIRS})

# Contains the current source code directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Set the source files and header files that need to be compiled
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
)

# Adding an executable file
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link related libraries
# - OpenCV: supports image processing, dnn, etc.
# - Qt5 Widgets: provides GUI support
# - GPIOD: supports GPIO operations
# - pthread: standard thread library (need to be linked under Linux)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    Qt5::Widgets
    ${GPIOD_LIBRARIES}
    pthread
)
