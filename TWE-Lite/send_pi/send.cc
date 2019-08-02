#include <iostream>

#define RASPBERRY_PI
#include "../TWE-Lite.hpp"

TWE_Lite twelite("/dev/ttyUSB0", 38400);

int main(int argc, char **argv){
	if(!twelite.init())
		exit(1);

//	twelite.send_buf[0] = 0x00;
	twelite.send_buf[0] = 0x22;

	while(true){
		twelite.send(2);
		std::cout << "send" << std::endl;
		delay(300);
	}
	return 0;
}
