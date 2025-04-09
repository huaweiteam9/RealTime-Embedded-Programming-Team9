//#include <opencv2/dnn.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
//#include <iostream>
//#include <vector>
//#include <string>
//
//using namespace std;
//using namespace cv;
//using namespace dnn;
//
//int main()
//{
//    // --------------------- �������� ---------------------
//    // ģ���ļ���ͼƬ�ļ�·���������ʵ������޸ģ�
//    string modelPath = "best.onnx";  // ������ YOLOv5s ONNX ģ��
//    string imagePath = "test7.jpg";   // �����ͼ��
//
//    // ��������ߴ磨YOLOv5s Ĭ������ߴ�Ϊ 640��640��
//    int inputWidth = 640;
//    int inputHeight = 640;
//
//    // ���Ŷ���ֵ
//    float confThreshold = 0.25f;
//    float nmsThreshold = 0.45f;
//
//    // --------------------- ����ģ�� ---------------------
//    Net net = readNetFromONNX(modelPath);
//    if (net.empty()) {
//        cerr << "����ģ��ʧ�ܣ�����·��: " << modelPath << endl;
//        return -1;
//    }
//    cout << "�ɹ�����ģ�ͣ�" << modelPath << endl;
//
//    // --------------------- ��ȡ��Ԥ����ͼƬ ---------------------
//    Mat img = imread(imagePath);
//    if (img.empty()) {
//        cerr << "�޷�����ͼƬ������·��: " << imagePath << endl;
//        return -1;
//    }
//
//    // Ԥ������һ����������С��ע�� swapRB Ϊ true��BGR ת RGB��
//    Mat blob;
//    blobFromImage(img, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
//    net.setInput(blob);
//
//    // --------------------- ִ��ǰ������ ---------------------
//    // �������ģ�����Ϊһ����������״ [1, 25200, 16]
//    vector<Mat> outs;
//    net.forward(outs, net.getUnconnectedOutLayersNames());
//
//    cout << "�����ܹ��õ� " << outs.size() << " �����" << endl;
//    for (size_t i = 0; i < outs.size(); ++i) {
//        cout << "----------------------------------" << endl;
//        cout << "��� " << i << ":" << endl;
//        cout << "ά������ " << outs[i].dims << endl;
//        for (int j = 0; j < outs[i].dims; ++j) {
//            cout << "    �� " << j << " ά��С: " << outs[i].size[j] << endl;
//        }
//        cout << "��Ԫ����: " << outs[i].total() << endl;
//        cout << "Channels: " << outs[i].channels() << endl;
//    }
//    cout << "----------------------------------" << endl;
//
//    // --------------------- ���ܲ�ȷ�������ʽ ---------------------
//    // �������Ϊ 3 ά��������״Ϊ [1, 25200, 16]
//    Mat outBlob = outs[0];
//    if (outBlob.dims != 3) {
//        cerr << "���������ά���쳣��Ԥ��Ϊ 3 ά����ʵ��Ϊ " << outBlob.dims << " ά" << endl;
//        return -1;
//    }
//    int batch = outBlob.size[0];       // ӦΪ1
//    int numPredictions = outBlob.size[1]; // ӦΪ25200
//    int numAttrs = outBlob.size[2];       // ӦΪ16
//
//    cout << "��� Blob ����״: [" << batch << ", " << numPredictions << ", " << numAttrs << "]" << endl;
//
//    // ���������Ϊ��ά����ÿһ��Ϊһ����ѡ��ÿ��Ϊ����
//    Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());
//    cout << "ת����ļ�����ߴ�: " << detectionMat.rows << " x " << detectionMat.cols << endl;
//
//    // ��ӡ��һ����ѡ������ݣ�����ȷ�ϸ�ʽ��
//    cout << "�� 0 ����ѡ������:" << endl;
//    for (int i = 0; i < detectionMat.cols; i++) {
//        cout << detectionMat.at<float>(0, i) << " ";
//    }
//    cout << endl;
//
//    // --------------------- ��������������Ƽ��� ---------------------
//    // ÿ����ѡ�����ݸ�ʽ��[cx, cy, w, h, obj_conf, score_0, score_1, ... , score_10]
//    vector<Rect> boxes;
//    vector<float> confidences;
//    vector<int> classIds;
//    for (int i = 0; i < detectionMat.rows; i++) {
//        float cx = detectionMat.at<float>(i, 0);
//        float cy = detectionMat.at<float>(i, 1);
//        float w = detectionMat.at<float>(i, 2);
//        float h = detectionMat.at<float>(i, 3);
//        float objectness = detectionMat.at<float>(i, 4);
//
//        // ȡ���÷֣����� 5~15�����ҳ����÷ּ���Ӧ���������
//        Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
//        Point classIdPoint;
//        double maxClassScore;
//        minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
//        float confidence = objectness * float(maxClassScore);
//
//        if (confidence > confThreshold) {
//            // ��������ٶ��߿���ֵ�� inputWidth*inputHeight �߶��£�640��640����ת����ԭͼ����
//            int left = int((cx - w / 2) * float(img.cols) / inputWidth);
//            int top = int((cy - h / 2) * float(img.rows) / inputHeight);
//            int right = int((cx + w / 2) * float(img.cols) / inputWidth);
//            int bottom = int((cy + h / 2) * float(img.rows) / inputHeight);
//            boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
//            confidences.push_back(confidence);
//            classIds.push_back(classIdPoint.x);
//        }
//    }
//
//    // �Ǽ���ֵ���ƣ��ų��ص�����ļ���
//    vector<int> indices;
//    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
//
//    // ����ʣ��ļ���
//    for (int idx : indices) {
//        Rect box = boxes[idx];
//        rectangle(img, box, Scalar(0, 255, 0), 2);
//        // �������ʾ������������Ŷȣ�ʵ�ʿ���������滻
//        string label = format("ID:%d %.2f", classIds[idx], confidences[idx]);
//        putText(img, label, box.tl(), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
//    }
//
//    // --------------------- ��ʾ���ͼ�� ---------------------
//    imshow("�����", img);
//    waitKey(0);
//
//    return 0;
//}



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
    // ģ���ļ���ͼƬ�ļ�·���������ʵ������޸ģ�
    string modelPath = "best.onnx";  // ������ YOLOv5s ONNX ģ��
    string imagePath = "test7.jpg";   // �����ͼ��

    // ��������ߴ磨YOLOv5s Ĭ������ߴ�Ϊ 640��640��
    int inputWidth = 640;
    int inputHeight = 640;

    // ���Ŷ���ֵ���Ǽ���ֵ������ֵ
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;

    // ��������б�˳���Ӧ��ѡ��Ԥ��ʱ�� 5~15 �е� 11 �����÷�
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

    // --------------------- ��ȡ��Ԥ����ͼƬ ---------------------
    Mat img = imread(imagePath);
    if (img.empty()) {
        cerr << "�޷�����ͼƬ������·��: " << imagePath << endl;
        return -1;
    }

    // Ԥ������һ����������С��swapRB ��Ϊ true��BGR ת RGB��
    Mat blob;
    blobFromImage(img, blob, 1.0 / 255.0, Size(inputWidth, inputHeight), Scalar(), true, false);
    net.setInput(blob);

    // --------------------- ִ��ǰ������ ---------------------
    // �������ģ�����Ϊһ����������״ [1, 25200, 16]
    vector<Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames());

    cout << "�����ܹ��õ� " << outs.size() << " �����" << endl;
    for (size_t i = 0; i < outs.size(); ++i) {
        cout << "----------------------------------" << endl;
        cout << "��� " << i << ":" << endl;
        cout << "ά������ " << outs[i].dims << endl;
        for (int j = 0; j < outs[i].dims; ++j) {
            cout << "    �� " << j << " ά��С: " << outs[i].size[j] << endl;
        }
        cout << "��Ԫ����: " << outs[i].total() << endl;
        cout << "Channels: " << outs[i].channels() << endl;
    }
    cout << "----------------------------------" << endl;

    // --------------------- ���ܲ�ȷ�������ʽ ---------------------
    // �������Ϊ 3 ά��������״Ϊ [1, 25200, 16]
    Mat outBlob = outs[0];
    if (outBlob.dims != 3) {
        cerr << "���������ά���쳣��Ԥ��Ϊ 3 ά����ʵ��Ϊ " << outBlob.dims << " ά" << endl;
        return -1;
    }
    int batch = outBlob.size[0];         // ӦΪ1
    int numPredictions = outBlob.size[1];  // ӦΪ25200
    int numAttrs = outBlob.size[2];        // ӦΪ16

    cout << "��� Blob ����״: [" << batch << ", " << numPredictions << ", " << numAttrs << "]" << endl;

    // ���������Ϊ��ά����ÿһ�ж�Ӧһ����ѡ��ÿһ��Ϊ����
    Mat detectionMat(numPredictions, numAttrs, CV_32F, outBlob.ptr<float>());
    cout << "ת����ļ�����ߴ�: " << detectionMat.rows << " x " << detectionMat.cols << endl;

    // ��ӡ��һ����ѡ������ݣ�����ȷ�ϸ�ʽ
    cout << "�� 0 ����ѡ������:" << endl;
    for (int i = 0; i < detectionMat.cols; i++) {
        cout << detectionMat.at<float>(0, i) << " ";
    }
    cout << endl;

    // --------------------- ��������������Ƽ��� ---------------------
    // ���ݸ�ʽ��[cx, cy, w, h, obj_conf, score_0, score_1, ... , score_10]
    vector<Rect> boxes;
    vector<float> confidences;
    vector<int> classIds;
    for (int i = 0; i < detectionMat.rows; i++) {
        float cx = detectionMat.at<float>(i, 0);
        float cy = detectionMat.at<float>(i, 1);
        float w = detectionMat.at<float>(i, 2);
        float h = detectionMat.at<float>(i, 3);
        float objectness = detectionMat.at<float>(i, 4);

        // ��ȡ���÷֣�score_0 �� score_10��
        Mat scores = detectionMat.row(i).colRange(5, detectionMat.cols);
        Point classIdPoint;
        double maxClassScore;
        minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
        float confidence = objectness * static_cast<float>(maxClassScore);

        if (confidence > confThreshold) {
            // ��������� bbox �������� 640x640 �߶��£���Ҫ����ӳ���ԭͼ�ߴ�
            int left = int((cx - w / 2) * float(img.cols) / inputWidth);
            int top = int((cy - h / 2) * float(img.rows) / inputHeight);
            int right = int((cx + w / 2) * float(img.cols) / inputWidth);
            int bottom = int((cy + h / 2) * float(img.rows) / inputHeight);
            boxes.push_back(Rect(Point(left, top), Point(right, bottom)));
            confidences.push_back(confidence);
            // classIdPoint.x Ϊ 0~10����Ӧ classNames �е� 11 ������
            classIds.push_back(classIdPoint.x);
        }
    }

    // �Ǽ���ֵ���ƣ��ų��ص��϶�ļ���
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    // ����ʣ����򼰱�ǩ��������������滻���֣�
    for (int idx : indices) {
        Rect box = boxes[idx];
        rectangle(img, box, Scalar(0, 255, 0), 2);
        string label;
        int classId = classIds[idx];
        if (classId >= 0 && classId < classNames.size())
            label = format("%s %.2f", classNames[classId].c_str(), confidences[idx]);
        else
            label = format("ID:%d %.2f", classId, confidences[idx]);
        // �ڱ߿�����Ͻǻ��Ʊ�ǩ�ı�
        putText(img, label, box.tl(), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
    }

    // --------------------- ��ʾ���ͼ�� ---------------------
    imshow("�����", img);
    waitKey(0);

    return 0;
}
