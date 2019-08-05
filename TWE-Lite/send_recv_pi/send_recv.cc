#include <iostream>
#define RASPBERRY_PI
#include "../TWE-Lite.hpp"

TWE_Lite twelite("/dev/ttyUSB0", 115200);

int main(int argc, char **argv){
	twelite.init();

	uint8_t buf[] = { 'A', 'B' };
	while(true){
		std::cout << "send: ";
//		twelite.recv();
		twelite.send_buf_extend(0x00, 0x01, buf, 2);
		if(twelite.check_send())
			std::cout << "ok";
		else
			std::cout << "fail";
		std::cout << std::endl;

		delay(100);
continue;
		const size_t size = twelite.recv();
		if(size == 0) continue;
		std::cout << "recv: ";
		for(size_t i=0;i<size;i++){
//			std::cout << std::hex << (uint32_t)twelite.recv_buf[i];
		}
		std::cout << std::endl;
	}

	return 0;
}
