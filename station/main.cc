#include <iostream>
#include <thread>
#include "twelite.hpp"

void print_vec(const twelite::vec_t &v);

int main(int argc, char **argv){
	std::cout << "initialize TWE-Lite...";
	if(twelite::init()){
		std::cout << "[ok]" << std::endl;
	}else{
		std::cout << "[failed]" << std::endl;
		return -1;
	}

	std::thread twelite_thread(twelite::loop);

	while(true){
		using namespace twelite;
		auto acc = latest_acc;
		auto gyro= latest_gyro;
		std::cout << "acc:  ";
		print_vec(acc);
		std::cout << "gyro: ";
		print_vec(gyro);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}

void print_vec(const twelite::vec_t &v){
	std::cout
		<< "time=" << v.time
		<< ", x=" << v.x
		<< ", y=" << v.y
		<< ", z=" << v.z << std::endl;
}
