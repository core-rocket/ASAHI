#include <iostream>
#define RASPBERRY_PI
#include "../TWE-Lite.hpp"

TWE_Lite twelite("/dev/ttyUSB0", 115200);

struct Hoge {
	uint8_t u8;
	uint16_t u16;
	char str[5];
}__attribute__((__packed__));

int main(int argc, char **argv){
	twelite.init();

	Hoge hoge;

	while(true){
		if(twelite.recv(hoge) != 0){
			std::cout
				<< "u8 = " << std::hex << (uint32_t)hoge.u8 << std::endl
				<< "u16=" << std::hex << hoge.u16 << std::endl
				<< "str:";
			for(size_t i=0;i<5;i++)
				std::cout << hoge.str[i];
			std::cout << std::endl;
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
