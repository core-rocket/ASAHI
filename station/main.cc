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
		}else if(cmd == "unlock"){
			// 縦解放機構ロック解除コマンド
			for(size_t i=0;i<3;i++)
				cmd_queue.push(0x02);
		}else if(cmd == "lock"){
			// 縦解放機構ロックコマンド
			for(size_t i=0;i<3;i++)
				cmd_queue.push(0x03);
		}else if(cmd == "on"){
			// フライトモードONコマンド
			for(size_t i=0;i<3;i++)
				cmd_queue.push(0x04);
		}else if(cmd == "motion" || cmd == "m"){
			auto acc = latest_acc;
			auto gyro= latest_gyro;
			std::cout << "acc:  ";
			print_vec(acc);
			std::cout << "gyro: ";
			print_vec(gyro);
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
	std::ofstream f_log;
	std::ofstream f_acc, f_gyro, f_bus_temp, f_gps_time, f_gps_pos, f_gps_alt;
	std::ofstream f_mission_temp, f_pressure, f_altitude;

	f_log.open("log/log.txt", std::ios::app);
	f_acc.open("log/acc.csv", std::ios::app);
	f_gyro.open("log/gyro.csv", std::ios::app);
	f_bus_temp.open("log/bus_temp.csv", std::ios::app);
	f_gps_time.open("log/gps_time.csv", std::ios::app);
	f_gps_pos.open("log/gps_pos.csv", std::ios::app);
	f_gps_alt.open("log/gps_alt.csv", std::ios::app);
	f_mission_temp.open("log/mission_temp.csv", std::ios::app);
	f_pressure.open("log/pressure.csv", std::ios::app);
	f_altitude.open("log/altitude.csv", std::ios::app);

	while(run_flag){
//		for(size_t i=0;i<log.size();i++){
//			const auto &s = log.front();
//			f_log << s << std::endl;
//			log.pop();
//		}

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

		for(size_t i=0;i<bus_temp.size();i++){
			const auto &t = bus_temp.front();
			f_bus_temp << t.time << "," << t.val << std::endl;
			bus_temp.pop();
		}

		for(size_t i=0;i<gps_time.size();i++){
			const auto &t = gps_time.front();
			f_gps_time << t.time << "," << t.time_int << "." << t.time_dec << std::endl;
			gps_time.pop();
		}
		for(size_t i=0;i<gps_pos.size();i++){
			const auto &p = gps_pos.front();
			f_gps_pos << p.time << "," << p.x_int << "." << p.x_dec << "," << p.y_int << "." << p.y_dec << std::endl;
			gps_pos.pop();
		}
		for(size_t i=0;i<gps_alt.size();i++){
			const auto &a = gps_alt.front();
			f_gps_alt << a.time << "," << a.x_int << "." << a.x_dec << "," << a.y_int << "." << a.y_dec << std::endl;
			gps_alt.pop();
		}

		// mission
		for(size_t i=0;i<mission_temp.size();i++){
			const auto &t = mission_temp.front();
			f_mission_temp << t.time << "," << t.val << std::endl;
			mission_temp.pop();
		}
		for(size_t i=0;i<pressure.size();i++){
			const auto &p = pressure.front();
			f_pressure << p.time << "," << p.val << std::endl;
			pressure.pop();
		}
		for(size_t i=0;i<altitude.size();i++){
			const auto &a = altitude.front();
			f_altitude << a.time << "," << a.val << std::endl;
			altitude.pop();
		}
	//	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
