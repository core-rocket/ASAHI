#define TWE_LITE_USE_HARDWARE_SERIAL
#include "../TWE-Lite.hpp"

TWE_Lite twelite(4, 3, 38400);

void setup(){
//	Serial.begin(38400);
//	Serial.println("setup");

	twelite.init();
	for(int i=0;i<80;i++)
		twelite.send_buf[i] = (uint8_t)i;
	twelite.send_buf[0] = 0x01; // なんか知らんが最初の1byteは0x00だと微妙っぽい(?)
}

// 送信するバイト数が増える程到達しにくくなる(要計測)

void loop(){
	static uint8_t buf[] = { 'A' };

	twelite.send_buf_extend(0x78, 0x01, buf, 1);
	if(twelite.check_send()){
		Serial.println("\nsend success");
	}else{
		Serial.println("\nsend failed");
	}
	delay(100);
}
