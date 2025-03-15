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


void OpenCamera() {

	while (1) {
	
		cout << "open camera successfully" << endl;
		

	}



}


void DetectObject() {
	while (1) {

		cout << "detect successfully" << endl;


	}


}


void DataCollecting() {
	while (1) {

		cout << "collect data successfully" << endl;


	}


}

void QTshowing() {
	while (1) {

		cout << "window shows successfully" << endl;


	}


}

void task5() {
	while (1) {

		cout << "task5 success" << endl;


	}


}

void task6() {
	while (1) {

		cout << "task6 success" << endl;


	}


}

int main(int, const char**) {

	// create pthread
	std::thread thread1(OpenCamera, 1);
	std::thread thread2(DetectObject, 2);
	std::thread thread3(DataCollecting, 3);
	std::thread thread4(QTshowing, 4);
	std::thread thread5(task5, 5);
	std::thread thread6(task6, 6);

	// wait 6 threads to complete
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	thread5.join();
	thread6.join();

	std::cout << "all threads are over " << std::endl;
	

	
	//pthread_t t1, t2, t3, t4, t5;

	//create thread 1
	//pthread_create£¨& t1, NULL,, NULL£©;



	//create thread 1
	//pthread_create£¨& t1, NULL, opencamera, NULL£©;

	//create thread 2 
	//pthread_create£¨& t2, NULL, opencamera, NULL£©;

	//pthread_join(t1, NULL);

	//pthread_join(t2, NULL);

	//return (0)






	return 0;
}