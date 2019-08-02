//#include <SoftwareSerial.h>
#include "../TWE-Lite.hpp"

TWE_Lite twelite(4, 3, 38400);

void setup(){
	Serial.begin(38400);
	Serial.println("setup");

	twelite.init();
	for(int i=0;i<80;i++)
		twelite.send_buf[i] = (uint8_t)i;
	twelite.send_buf[0] = 0x01; // なんか知らんが最初の1byteは0x00だと微妙っぽい(?)
}

// 送信するバイト数が増える程到達しにくくなる(要計測)

void loop(){
	static int i = 0;

	Serial.print("loop ");

	int num = (i % 80) + 1;
	Serial.print(num);

	twelite.send(1);
	delay(100);
	for(int k=0;k<3;k++){
		twelite.send(static_cast<uint8_t>(num));
		delay(10);
	}
	twelite.send(80);
	delay(1000);
	i++;
}
