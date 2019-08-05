#define TWE_LITE_USE_HARDWARE_SERIAL
#include "../TWE-Lite.hpp"

TWE_Lite twelite(4, 3, 38400);

void setup(){
	Serial.begin(38400);
	Serial.println("setup");

	twelite.init();
}

// 送信するバイト数が増える程到達しにくくなる(要計測)

void loop(){
	static size_t num = 0;
	static uint8_t buf[] = { 'A' };

	if(num % 2 == 0)
		twelite.send_buf_simple(0x01, 0x02, buf, 1);
	else
		twelite.send_buf_extend(0x01, 0x01, buf, 1);

	// response message check
check:
	auto ret = twelite.check_send();
	if(ret < 0) goto check;

	if(ret){
		Serial.println("\r\nsend success");
	}else{
		Serial.println("\r\nsend failed");
	}

	num++;
	delay(300);
}
