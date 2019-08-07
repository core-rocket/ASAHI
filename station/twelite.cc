#include <iostream>

#define RASPBERRY_PI
#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.h"

#include "twelite.hpp"

TWE_Lite twe("/dev/ttyUSB0", 115200);

void parse_simple(const TWE_Lite &twe);
void parse_extend(const TWE_Lite &twe);

namespace twelite {
	vec_t latest_acc = {};
	vec_t latest_gyro= {};
}

bool twelite::init(){
	return twe.init();
}

void twelite::loop(){
	while(true){
		// 送信処理

		// 受信
		if(twe.recv() == 0) continue;

		if(twe.is_response()){
			if(twe.recv_buf[0] == 0x01){
				// 送信成功
			}else{
				// 送信失敗
			}
			continue;
		}

		if(twe.is_simple())
			parse_simple(twe);
		else
			parse_extend(twe);

		delay(1);
	}
}

void get_string(const TWE_Lite &twe){
	std::cout << "string: \"";
	for(size_t i=0;i<twe.get_length();i++){
		std::cout << static_cast<char>(twe.recv_buf[i]);
	}
	std::cout << std::endl;
}

void get_acc(const TWE_Lite &twe){
	auto *acc = twe.get_data<Vec16_t>();
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
}

void get_gyro(const TWE_Lite &twe){
	auto *gyro = twe.get_data<Vec16_t>();
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
}

// 簡易形式で受信したデータをパースする
void parse_simple(const TWE_Lite &twe){
	switch(twe.cmd_type()){
		case 0x00:	// 文字列
			get_string(twe);
			break;
		case 0x01:	// 3軸加速度
			get_acc(twe);
			break;
		case 0x02:	// 3軸ジャイロ
			get_gyro(twe);
			break;
		default:
			std::cout
				<< "unknown type data(simple format)" << std::endl
				<< "\tcmd_type = 0x" << std::hex << static_cast<uint32_t>(twe.cmd_type()) << std::endl
				<< "\tlength = " << std::dec << twe.get_length() << std::endl;
	}
}

void parse_extend(const TWE_Lite &twe){
	std::cout << "parse_extend " << std::endl;
}
