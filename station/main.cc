#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include "twelite.hpp"

bool run_flag;

void print_vec(const twelite::vec_t &v);

void save_loop();

int main(int argc, char **argv){
	std::cout << "initialize TWE-Lite...";
	if(twelite::init()){
		std::cout << "[ok]" << std::endl;
	}else{
		std::cout << "[failed]" << std::endl;
		return -1;
	}

	run_flag = true;

	std::thread twelite_thread(twelite::loop);
	std::thread save_thread(save_loop);

	std::string cmd;

	while(true){
		using namespace twelite;

		std::cout << "> ";
		std::cin >> cmd;

		if(cmd == "exit"){	// 雑
			break;
		}else if(cmd == "motion" || cmd == "m"){
			auto acc = latest_acc;
			auto gyro= latest_gyro;
			std::cout << "acc:  ";
			print_vec(acc);
			std::cout << "gyro: ";
			print_vec(gyro);
		}else if(cmd == "unlock"){
			for(size_t i=0;i<3;i++)
				cmd_queue.push(0x02);
		}
	}

	run_flag = false;

	twelite_thread.join();
	save_thread.join();

	return 0;
}

void print_vec(const twelite::vec_t &v){
	std::cout
		<< "time=" << v.time
		<< ", x=" << v.x
		<< ", y=" << v.y
		<< ", z=" << v.z << std::endl;
}

void fwrite_vec(std::ofstream &file, const twelite::vec_t &v){
	file << v.time << "," << v.x << "," << v.y << "," << v.z << std::endl;
}

void save_loop(){
	using namespace twelite;
	std::ofstream f_acc, f_gyro;

	f_acc.open("log/acc.csv", std::ios::app);
	f_gyro.open("log/gyro.csv", std::ios::app);

	while(run_flag){
		for(size_t i=0;i<acc.size();i++){
			const auto &v = acc.front();
			fwrite_vec(f_acc, v);
			acc.pop();
		}
		for(size_t i=0;i<gyro.size();i++){
			const auto &v = gyro.front();
			fwrite_vec(f_gyro, v);
			gyro.pop();
		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
