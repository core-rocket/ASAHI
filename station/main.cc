#include <iostream>
#define RASPBERRY_PI
#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.h"

TWE_Lite twe("/dev/ttyUSB0", 115200);

void parse_simple(const TWE_Lite &twe);
void parse_extend(const TWE_Lite &twe);

int main(int argc, char **argv){
//	TWE_Lite twe("/dev/ttyUSB0", 115200);

	std::cout << "initialize TWE-Lite...";
	if(twe.init()){
		std::cout << "[ok]" << std::endl;
	}else{
		std::cout << "[failed]" << std::endl;
		return -1;
	}

	// 受信ループ
	while(true){
		if(twe.recv() == 0) continue; // 受信失敗

		if(twe.is_response()){ // 応答メッセージ
			if(twe.recv_buf[0] == 0x01){
				// 送信成功
				std::cout << "send success" << std::endl;
			}else{
				// 送信失敗
				std::cout << "send failed" << std::endl;
			}
			continue;
		}

		if(twe.is_simple())	// 簡易形式
			parse_simple(twe);
		else				// 拡張形式
			parse_extend(twe);

		delay(1); // 少し待った方がいい
	}

	return 0;
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
	std::cout << "acc: ";
	if(acc == nullptr){
		std::cout << "error" << std::endl;
		return;
	}

	float v[3];
	for(int i=0;i<3;i++)
		v[i] = static_cast<float>(acc->raw[i]) / 16384.0;
	std::cout
		<< "x=" << v[0]
		<< ",\ty=" << v[1]
		<< ",\tz=" << v[2] << std::endl;
}

void get_gyro(const TWE_Lite &twe){
	auto *gyro = twe.get_data<Vec16_t>();
	std::cout << "gyro: ";
	if(gyro == nullptr){
		std::cout << "error" << std::endl;
		return;
	}

	float v[3];
	for(int i=0;i<3;i++)
		v[i] = static_cast<float>(gyro->raw[i]) / 131.0;
	std::cout
		<< "x=" << v[0]
		<< ",\ty=" << v[1]
		<< ",\tz=" << v[2] << std::endl;
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
