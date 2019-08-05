#include <iostream>
#define RASPBERRY_PI
#include "../TWE-Lite.hpp"

TWE_Lite twelite("/dev/ttyUSB0", 115200);

int main(int argc, char **argv){
	twelite.init();

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
