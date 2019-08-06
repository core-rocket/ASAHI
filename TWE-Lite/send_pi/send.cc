#include <iostream>

#define RASPBERRY_PI
#include "../TWE-Lite.hpp"

TWE_Lite twelite("/dev/ttyUSB0", 115200);

int main(int argc, char **argv){
	if(!twelite.init())
		exit(1);

	uint8_t buf[] = { 'A', 'B' };

	while(true){
		twelite.send_buf_simple(0x00, 0x01, buf, 2);
		std::cout << std::endl;
		std::cout << "send: ";
		if(twelite.check_send())
			std::cout << "ok";
		else
			std::cout << "failed";
		std::cout << std::endl;

		while(twelite.savail())
			twelite.sread8();
		delay(100);
	}
	return 0;
}
