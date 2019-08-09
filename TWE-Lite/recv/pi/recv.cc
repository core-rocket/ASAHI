#include <iostream>
#include <string>

#include <wiringPi.h>
#include <wiringSerial.h>

class TWE_Lite {
public:
	TWE_Lite(const std::string devfile="/dev/ttyUSB0", const size_t brate=115200) : devfile(devfile), brate(brate) {}

	bool init(){
		if((fd = serialOpen(devfile.c_str(), brate)) < 0)
			return false;
		return true;
	}

	const char get_char(){
		while(!serialDataAvail(fd));
		return serialGetchar(fd);
	}
private:
	const std::string devfile;
	const size_t brate;
	int fd;
};

int main(int argc, char **argv){
	TWE_Lite twe("/dev/ttyUSB0", 38400);
	twe.init();

	while(true){
		std::cout << twe.get_char() << std::endl;
	}
	return 0;
}
