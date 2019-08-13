#include <iostream>
#define RASPBERRY_PI
#include "../TWE-Lite.hpp"

TWE_Lite twelite("/dev/ttyUSB0", 115200);

struct Hoge {
	uint8_t u8;
	uint16_t u16;
	char str[5];
}__attribute__((__packed__));

struct Fuga {
	float vec[3];
}__attribute__((__packed__));

struct Motion {
	float acc[1];
}__attribute__((__packed__));

int main(int argc, char **argv){
	twelite.init();

	while(true){
		if(twelite.recv() == 0) continue;

		if(!twelite.is_simple()) continue;

		if(twelite.cmd_type() == 0x00){
			std::cout
				<< "recieve string" << std::endl
				<< "\t" << twelite.recv_buf << std::endl;
		}else if(twelite.cmd_type() == 0x02){
			auto *hoge = twelite.get_data<Hoge>();
			if(hoge == nullptr)
				continue;
			std::cout
				<< "recieve Hoge" << std::endl
				<< "\tu8 = " << std::hex << (uint32_t)hoge->u8 << std::endl
				<< "\tu16=" << std::hex << hoge->u16 << std::endl
				<< "\tstr:";
			for(size_t i=0;i<5;i++)
				std::cout << hoge->str[i];
			std::cout << std::endl;

			twelite.clear_buf();
		}else if(twelite.cmd_type() == 0x03){
			auto *fuga = twelite.get_data<Fuga>();
			if(fuga == nullptr)
				continue;
			std::cout
				<< "recieve Fuga" << std::endl
				<< "\tvec[0]" << fuga->vec[0] << std::endl
				<< "\tvec[1]" << fuga->vec[1] << std::endl
				<< "\tvec[2]" << fuga->vec[2] << std::endl;
		}else if(twelite.cmd_type() == 0x04){
			auto *motion = twelite.get_data<Motion>();
			if(motion == nullptr) continue;
			std::cout
				<< "recieve motion" << std::endl
				<< "\tacc[0]" << motion->acc[0] << std::endl;
		}else{
			std::cout << "recieve(" << std::hex << (uint32_t)twelite.cmd_type() << ")" << std::endl;
		}
	}


	while(true){
		size_t size = twelite.recv();
		if(size == 0) continue;
		std::cout << "recv:" << std::endl
			<< "\tfrom = " << std::dec << (uint32_t)twelite.from_id() << std::endl
			<< "\tpayload: ";
		for(size_t i=0;i<size;i++){
			std::cout << std::hex << (uint32_t)twelite.recv_buf[i];
		}
		std::cout << std::endl;
		if(twelite.is_extended())
			std::cout << "\tLQI = " << std::dec << (uint32_t)twelite.LQI();
		std::cout << std::endl << std::endl;
	}
}
