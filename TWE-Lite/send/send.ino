#define TWE_LITE_USE_HARDWARE_SERIAL
#include "../TWE-Lite.hpp"

TWE_Lite twelite(4, 3, 38400);

void setup(){
//	Serial.begin(38400);
//	Serial.println("setup");

	twelite.init();
}

// 送信するバイト数が増える程到達しにくくなる(要計測)

void loop(){
	static size_t num = 0;
	static uint8_t buf[] = { 'A' };

	if(num % 2 == 0)
		twelite.send_buf_simple(0x78, 0x01, buf, 1);
	else
		twelite.send_buf_extend(0x78, 0x01, buf, 1);
	if(twelite.check_send()){
		Serial.println("\r\nsend success");
	}else{
		Serial.println("\r\nsend failed");
	}

	num++;
	delay(100);
}
