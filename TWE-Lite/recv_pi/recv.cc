#include <iostream>
#define RASPBERRY_PI
#include "../TWE-Lite.hpp"

TWE_Lite twelite("/dev/ttyUSB0", 38400);

int main(int argc, char **argv){
	twelite.init();

	while(true){
		size_t size = twelite.recv();
		for(size_t i=0;i<size;i++){
			std::cout << std::hex << (uint32_t)twelite.recv_buf[i];
		}
		std::cout << std::endl;
	}
/*
	std::cout << (int)twelite.parser.get_state() << std::endl;
	while(true){
		uint8_t b = twelite.sread8();
		bool flg = twelite.parser.parse8(b);
//		std::cout << (int)twelite.parser.get_state() << " "
//			<< std::hex << (int)b <<std::endl;

		auto error = twelite.parser.get_error();
		if(error != TWE_Lite::Parser::state::empty){
			std::cout << "error(" << (int)error << ")" << std::endl;
		}

		if(twelite.parser.get_state() == TWE_Lite::Parser::state::payload)
			std::cout << "length: " << std::dec << twelite.parser.get_length() << std::endl;

		if(flg){
			std::cout << "read complete" << std::endl
				<< "length: " << std::dec << twelite.parser.get_length() << std::endl
				<< "payload: ";
			for(uint16_t i=0;i<twelite.parser.get_length();i++)
				std::cout << std::hex << (int) twelite.recv_buf[i];
			std::cout << std::endl;
		}
	}
*/
}
