#include <stdio.h>
#include "CppTimer.h"
#include <unistd.h>
#include <time.h>
#include <thread>
#include <iostream>


using namespace std;

class DemoTimer1 : public CppTimer {

	void timerEvent() {
		


	}
};


class DemoTimer2 : public CppTimer {

	void timerEvent() {
		


	}
};



class DemoTimer3 : public CppTimer {

	void timerEvent() {
		


	}
};


bool stopThreads = false;

void OpenCamera() {
	while (!stopThreads) {
		cout << "open camera successfully" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void DetectObject() {
	while (!stopThreads) {
		cout << "detect successfully" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void DataCollecting() {
	while (!stopThreads) {
		cout << "collect data successfully" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void QTshowing() {
	while (!stopThreads) {
		cout << "window shows successfully" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void task5() {
	while (!stopThreads) {
		cout << "task5 success" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void task6() {
	while (!stopThreads) {
		cout << "task6 success" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

int main() {
	// 创建线程
	std::thread thread1(OpenCamera);
	std::thread thread2(DetectObject);
	std::thread thread3(DataCollecting);
	std::thread thread4(QTshowing);
	std::thread thread5(task5);
	std::thread thread6(task6);

	// 运行10秒后停止所有线程
	std::this_thread::sleep_for(std::chrono::seconds(10));
	stopThreads = true;

	// 等待所有线程结束
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	thread5.join();
	thread6.join();

	std::cout << "all threads are over" << std::endl;
	return 0;
}