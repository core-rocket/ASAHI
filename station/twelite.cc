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
	double_t latest_bus_temp = {};
	double_t latest_mission_temp = {};
	double_t latest_pressure = {};
	double_t latest_altitude = {};
	GPS_time latest_gps_time = {};
	GPS_vec2 latest_gps_pos = {};
	GPS_vec2 latest_gps_alt = {};

	std::queue<vec_t> acc, gyro;
	std::queue<uint8_t> cmd_queue;
	std::queue<std::string> log;
	std::queue<double_t> bus_temp;
	std::queue<double_t> mission_temp;
	std::queue<double_t> pressure;
	std::queue<double_t> altitude;
	std::queue<GPS_time> gps_time;
	std::queue<GPS_vec2> gps_pos;
	std::queue<GPS_vec2> gps_alt;
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
			twe->send_extend(id_mission, cmd_queue.front(), "A");
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
	std::string s = "str: ";
	std::cout << "string: \"";
	for(size_t i=0;i<twe->get_length();i++){
		std::cout << static_cast<char>(twe->recv_buf[i]);
		s += twe->recv_buf[i];
	}
	std::cout << "\"" << std::endl;

	twelite::log.push(s);
}

void get_acc(const TWE_Lite *twe){
	auto *acc = twe->get_data<Vec16_t>();
	if(acc == nullptr){
		return;
	}

	float t;
	t = static_cast<float>(acc->time) / 1000.0f;

	float v[3];
	for(int i=0;i<3;i++)
		v[i] = static_cast<float>(acc->raw[i]) / 4096.0; //16384.0;

	twelite::latest_acc = {
		t,
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

	float t;
	t = static_cast<float>(gyro->time) / 1000.0f;

	float v[3];
	for(int i=0;i<3;i++)
		v[i] = static_cast<float>(gyro->raw[i]) / 65.5; //131.0;

	twelite::latest_gyro = {
		t,
		v[0],
		v[1],
		v[2]
	};
	twelite::gyro.push(twelite::latest_gyro);
}

void get_bus_temp(const TWE_Lite *twe){
	auto *temp = twe->get_data<Value16>();
	if(temp == nullptr)
		return;

	float time = static_cast<float>(temp->time) / 1000.0f;
	double val= ((static_cast<double>(temp->val)+12412.0)/340.0);
	twelite::latest_bus_temp = {
		time,
		val
	};
	twelite::bus_temp.push(twelite::latest_bus_temp);
//	std::cout << "bus temperature: " << val << std::endl;
}

void get_mission_temp(const TWE_Lite *twe){
	auto *temp = twe->get_data<Float32>();
	if(temp == nullptr)
		return;

	float time = static_cast<float>(temp->time) / 1000.0f;
	double val = static_cast<double>(temp->value);
	twelite::latest_mission_temp = {
		time,
		val,
	};
	twelite::mission_temp.push(twelite::latest_mission_temp);

//	std::cout << "mission temp = " << val << std::endl;
}

void get_pressure(const TWE_Lite *twe){
	auto *press = twe->get_data<Float32>();
	if(press == nullptr)
		return;

	float time = static_cast<float>(press->time) / 1000.0f;
	double val = static_cast<double>(press->value);
	twelite::latest_pressure = {
		time,
		val,
	};
	twelite::pressure.push(twelite::latest_pressure);

//	std::cout << "mission pressure = " << val << std::endl;
}

void get_altitude(const TWE_Lite *twe){
	auto *alt = twe->get_data<Float32>();
	if(alt == nullptr)
		return;

	float time = static_cast<float>(alt->time) / 1000.0f;
	double val = static_cast<double>(alt->value);
	twelite::latest_altitude = {
		time,
		val,
	};
	twelite::altitude.push(twelite::latest_altitude);

//	std::cout << "mission altitude = " << val << std::endl;
}

void get_gps(const TWE_Lite *twe){
	switch(twe->cmd_type()){
		case 0x07:
			std::cout << "GPS flag" << std::endl;
			break;
		case 0x08:
			//std::cout << "GPS time" << std::endl;
			{
				auto *time = twe->get_data<GPS_time>();
				if(time == nullptr)
					return;
			//	std::cout << "UTC: "
			//		<< time->time_int << "." << time->time_dec << std::endl;
				twelite::latest_gps_time = {
					time->time,
					time->time_int,
					time->time_dec
				};
				twelite::gps_time.push(twelite::latest_gps_time);
			}
			break;
		case 0x09:
			//std::cout << "GPS pos" << std::endl;
			{
				auto *pos = twe->get_data<GPS_vec2>();
				if(pos == nullptr)
					return;
			//	std::cout
			//		<< "lat = " << pos->x_int << "." << pos->x_dec
			//		<< ", lng = " << pos->y_int << "." << pos->y_dec << std::endl;
				twelite::latest_gps_pos = {
					pos->time,
					pos->x_int, pos->x_dec,
					pos->y_int, pos->y_dec
				};
				twelite::gps_pos.push(twelite::latest_gps_pos);
			}
			break;
		case 0x0a:
			//std::cout << "GPS altitude" << std::endl;
			{
				auto *alt = twe->get_data<GPS_vec2>();
				if(alt == nullptr)
					return;
//				std::cout
//					<< "alt = " << alt->x_int << "." << alt->x_dec
//					<< ", alt geo = " << alt->y_int << "." << alt->y_dec << std::endl;
			}
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
		case 0x03:	// 水密部温度
			get_bus_temp(twe);
			break;
		case 0x04:	// 気温
			get_mission_temp(twe);
			break;
		case 0x05:	// 気圧
			get_pressure(twe);
			break;
		case 0x06:	// 高度
			get_altitude(twe);
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
	std::string s = "ex: ";
//	std::cout << "parse_extend("
//		<< static_cast<uint32_t>(twe->response_id())
//		<< ")"
//		<< std::endl;

	s += "LQI=";
	s += std::to_string(static_cast<uint32_t>(twe->LQI()));
	s += ", ";
	switch(twe->response_id()){
		case 0x00:	// bus status
			s += "status(bus) = ";
			s += std::to_string(static_cast<uint32_t>(twe->recv_buf[0]));
		//	std::cout
		//		<< "bus status = " << std::dec << static_cast<uint32_t>(twe->recv_buf[0]) << std::endl;
			break;
		case 0x01:
			s += "status(mission) = ";
			//s += std::to_string(static_cast<uint32_t>(twe->recv_buf[0]));
			switch(twe->recv_buf[0]){
			case 0x00: s += "standby"; break;
			case 0x01: s += "flight"; break;
			case 0x02: s += "rising"; break;
			case 0x03: s += "parachute"; break;
			case 0x04: s += "leafing"; break;
			}
		//	std::cout
		//		<< "mission status = " << std::dec << static_cast<uint32_t>(twe->recv_buf[0]) << std::endl;
			break;
		default:
			s += "unknown: response_id=";
			s += std::to_string(static_cast<uint32_t>(twe->response_id()));
			s += ", length=";
			s += std::to_string(twe->get_length());
		//	std::cout
		//		<< "unknown type data(extend format)" << std::endl
		//		<< "\tresponse_id = 0x" << std::hex << static_cast<uint32_t>(twe->response_id()) << std::endl
		//		<< "\tlength = " << std::dec << twe->get_length() << std::endl;
			break;
	}

	std::cout << s << std::endl;

	twelite::log.push(s);
}
