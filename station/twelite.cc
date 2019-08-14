#include <iostream>
#include <thread>

//#define RASPBERRY_PI
#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.hpp"

#include "twelite.hpp"

void parse_simple(const TWE_Lite *twe);
void parse_extend(const TWE_Lite *twe);

extern bool run_flag;

namespace twelite {
	TWE_Lite *twe = nullptr;
	vec_t latest_acc = {};
	vec_t latest_gyro= {};
	std::queue<vec_t> acc, gyro;
	std::queue<uint8_t> cmd_queue;
}

bool twelite::init(){
	twe = new TWE_Lite("/dev/ttyUSB0", 115200);
	return twe->init();
}

void twelite::loop(){
	while(run_flag){
		// 送信
		if(!cmd_queue.empty()){
			std::cout << "sending command ... \t";
			twe->send_extend(id_bus, cmd_queue.front(), "A");
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			cmd_queue.pop();
		}

		// 受信
		if(twe->recv(100) == 0) continue;

		if(twe->is_response()){
			if(twe->recv_buf[0] == 0x01){
				// 送信成功
				std::cout << "[success: " << (int)twe->response_id() << "]" << std::endl;
			}else{
				// 送信失敗
				std::cout << "[failed]" << std::endl;
			}
			continue;
		}

		if(twe->is_simple())
			parse_simple(twe);
		else
			parse_extend(twe);

		//delay(1);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void get_string(const TWE_Lite *twe){
	std::cout << "string: \"";
	for(size_t i=0;i<twe->get_length();i++){
		std::cout << static_cast<char>(twe->recv_buf[i]);
	}
	std::cout << "\"" << std::endl;
}

void get_acc(const TWE_Lite *twe){
	auto *acc = twe->get_data<Vec16_t>();
	if(acc == nullptr){
		return;
	}

	float v[3];
	for(int i=0;i<3;i++)
		v[i] = static_cast<float>(acc->raw[i]) / 16384.0;

	twelite::latest_acc = {
		acc->time,
		v[0],
		v[1],
		v[2]
	};
	twelite::acc.push(twelite::latest_acc);
}

void get_gyro(const TWE_Lite *twe){
	auto *gyro = twe->get_data<Vec16_t>();
	if(gyro == nullptr){
		return;
	}

	float v[3];
	for(int i=0;i<3;i++)
		v[i] = static_cast<float>(gyro->raw[i]) / 131.0;

	twelite::latest_gyro = {
		gyro->time,
		v[0],
		v[1],
		v[2]
	};
	twelite::gyro.push(twelite::latest_gyro);
}

void get_gps(const TWE_Lite *twe){
	switch(twe->cmd_type()){
		case 0x09:
			{
				auto *t = twe->get_data<GPS_time>();
				if(t == nullptr)
					return;
				std::cout << "GPS time: " << t->time_int << "." << t->time_dec << std::endl;
			}
			break;
		case 0x0a:
			{
				auto *p = twe->get_data<GPS_pos>();
				if(p == nullptr)
					return;
				std::cout << "GPS pos: "
					<< "lat = " << p->lat_int << "." << p->lat_dec
					<< ", lng = " << p->lng_int << "." << p->lng_dec
					<< std::endl;
			}
			break;
		default:
			std::cout << "unknown GPS data type" << std::endl;
			break;
	}
}

// 簡易形式で受信したデータをパースする
void parse_simple(const TWE_Lite *twe){
	switch(twe->cmd_type()){
		case 0x00:	// 文字列
			get_string(twe);
			break;
		case 0x01:	// 3軸加速度
			get_acc(twe);
			break;
		case 0x02:	// 3軸ジャイロ
			get_gyro(twe);
			break;
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
			get_gps(twe);
			break;
		default:
			std::cout
				<< "unknown type data(simple format)" << std::endl
				<< "\tcmd_type = 0x" << std::hex << static_cast<uint32_t>(twe->cmd_type()) << std::endl
				<< "\tlength = " << std::dec << twe->get_length() << std::endl;
	}
}

void parse_extend(const TWE_Lite *twe){
	std::cout << "parse_extend " << std::endl;
}
