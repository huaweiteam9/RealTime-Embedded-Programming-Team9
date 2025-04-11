#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace cv;
using namespace dnn;

int main()
{
    // --------------------- �������� ---------------------
    // ģ���ļ�·���������ʵ������޸ģ�
    string modelPath = "best.onnx";  // ������ YOLOv5s ONNX ģ��
    // ����ͷID��Ĭ��Ϊ0
    int cameraID = 0;

    // ��������ߴ磨YOLOv5s Ĭ������Ϊ 640��640��
    int inputWidth = 640;
    int inputHeight = 640;

    // ���Ŷ���ֵ���Ǽ���ֵ������ֵ
    float confThreshold = 0.7f;
    float nmsThreshold = 0.45f;

    // ��������б���Ӧ�����ѡ���е÷ֵ� 11 �����
    vector<string> classNames = {
        "apple", "cabbage", "carrot", "grape", "lemon",
        "mango", "napa cabbage", "peach", "pepper", "potato",
        "radish"
    };

    // --------------------- ����ģ�� ---------------------
    Net net = readNetFromONNX(modelPath);
    if (net.empty()) {
        cerr << "����ģ��ʧ�ܣ�����·��: " << modelPath << endl;
        return -1;
    }
    cout << "�ɹ�����ģ�ͣ�" << modelPath << endl;

    // --------------------- ������ͷ ---------------------
    VideoCapture cap(cameraID);
    if (!cap.isOpened()) {
        cerr << "�޷�������ͷ����������ͷ�豸ID������״̬��" << endl;
        return -1;
    }

    // ��������ͷ�ֱ��ʣ��ɸ�����Ҫ���ã�
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    // ��������
    const string windowName = "ʵʱ���";
    namedWindow(windowName, WINDOW_NORMAL);

    // --------------------- ʵʱ���ѭ�� ---------------------
    Mat frame;
    while (true) {
        // ��ȡ��ǰ֡
        cap >> frame;
        if (frame.empty()) {
            cerr << "��֡��������⡣" << endl;
            break;
        }

        // ����ԭͼ���ڻ��ƣ������Ҫ����ԭʼ֡��
        Mat outputFrame = frame.clone();

        // --------------------- ͼ��Ԥ���� ---------------------
        Mat blob;
        // ��֡����Ϊ 640x640����һ���� [0,1]��BGR -> RGB
        blobFromImage(frame, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
        net.setInput(blob);

        // --------------------- ǰ������ ---------------------
        vector<Mat> outs;
        net.forward(outs, net.getUnconnectedOutLayersNames());

        // �ٶ�ģ�����Ϊ 3 ά��������״ [1, 25200, 16]
        if (outs.empty()) {
            cerr << "����������" << endl;
            continue;
        }
        Mat outBlob = outs[0];
        if (outBlob.dims != 3) {
            cerr << "�������ά���쳣��Ԥ��Ϊ 3 ά����ʵ��Ϊ " << outBlob.dims << " ά" << endl;
            continue;
        }
        int numPredictions = outBlob.size[1];  // ���� 25200
        int numAttrs = outBlob.size[2];        // ���� 16

        // ���������Ϊ��ά����ÿ�д���һ����ѡ��
        Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());

        // --------------------- ��������� ---------------------
        vector<Rect> boxes;
        vector<float> confidences;
        vector<int> classIds;
        for (int i = 0; i < detectionMat.rows; i++) {
            float cx = detectionMat.at<float>(i, 0);
            float cy = detectionMat.at<float>(i, 1);
            float w = detectionMat.at<float>(i, 2);
            float h = detectionMat.at<float>(i, 3);
            float objectness = detectionMat.at<float>(i, 4);

            // ��ȡ���������������÷�λ������ 5 �� 15����11�����
            Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
            Point classIdPoint;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
            float confidence = objectness * static_cast<float>(maxClassScore);

            if (confidence > confThreshold) {
                // ����������Ϳ�ߴ� 640x640 �߶�ӳ�䵽ԭͼ�ߴ�
                int left = int((cx - w / 2) * float(frame.cols) / inputWidth);
                int top = int((cy - h / 2) * float(frame.rows) / inputHeight);
                int right = int((cx + w / 2) * float(frame.cols) / inputWidth);
                int bottom = int((cy + h / 2) * float(frame.rows) / inputHeight);
                boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
                confidences.push_back(confidence);
                classIds.push_back(classIdPoint.x);
            }
        }

        // --------------------- �Ǽ���ֵ���� ---------------------
        vector<int> indices;
        NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

        // --------------------- ���Ƽ���ͱ�ǩ ---------------------
        for (int idx : indices) {
            Rect box = boxes[idx];
            rectangle(outputFrame, box, Scalar(0, 255, 0), 2);
            string label;
            int clsId = classIds[idx];
            if (clsId >= 0 && clsId < classNames.size())
                label = format("%s %.2f", classNames[clsId].c_str(), confidences[idx]);
            else
                label = format("ID:%d %.2f", clsId, confidences[idx]);
            putText(outputFrame, label, box.tl(), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
        }

        // --------------------- ��ʾ��� ---------------------
        imshow(windowName, outputFrame);

        // �� 'q' ���˳�ʵʱ���
        char c = (char)waitKey(1);
        if (c == 'q' || c == 27)  // 'q' �� ESC ���˳�
            break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
