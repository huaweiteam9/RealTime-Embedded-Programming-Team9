cmake_minimum_required(VERSION 3.16)

project(qt_yolo VERSION 1.0 LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find Qt (assumes Qt5)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


# Add ONNX Runtime (modify this path to match your environment)
# set(ONNXRUNTIME_DIR "/path/to/onnxruntime") # optionally define if not system-installed
find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_c_api.h)
find_library(ONNXRUNTIME_LIB onnxruntime)

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIB)
    message(FATAL_ERROR "ONNX Runtime not found. Please install it and set ONNXRUNTIME_INCLUDE_DIR / ONNXRUNTIME_LIB.")
endif()

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    fruitvegdetect.cpp
    secondpage.cpp
)

set(HEADERS
    mainwindow.h
    fruitvegdetect.h
    secondpage.h
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Link Qt and ONNX Runtime
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${ONNXRUNTIME_LIB}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${ONNXRUNTIME_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
