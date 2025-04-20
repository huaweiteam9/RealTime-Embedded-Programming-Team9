#ifndef FRUITVEGDETECT_H
#define FRUITVEGDETECT_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

#include <QMap>
#include <QString>

using namespace cv;
using namespace dnn;
using namespace std;

/*
 * The FruitVegDetect class encapsulates fruit and vegetable detection functionality using a YOLO model,
 * including opening the camera, loading the model, capturing frames,
 * performing detection on images, and drawing detection results.
 */
class FruitVegDetect
{
public:
    // Constructor: takes model file path, camera ID, input size, confidence threshold, and NMS threshold
    FruitVegDetect(const std::string& modelPath, int cameraID = 0,
        int inputWidth = 640, int inputHeight = 640,
        float confThreshold = 0.7f, float nmsThreshold = 0.45f);
    ~FruitVegDetect();

    // Get the current detection frame; returns true if successful and stores result in frame, otherwise false
    bool getDetectionFrame(Mat& frame, QMap<QString, int>& detectionResults);
private:
    VideoCapture cap;     // Camera capture object
    Net net;              // DNN network model
    int inputWidth;       // Model input width
    int inputHeight;      // Model input height
    float confThreshold;  // Confidence threshold
    float nmsThreshold;   // Non-maximum suppression threshold
    vector<string> classNames; // List of class names
};

#endif // FRUITVEGDETECT_H
