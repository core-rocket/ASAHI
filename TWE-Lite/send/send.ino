#define TWE_LITE_USE_HARDWARE_SERIAL
#include "../TWE-Lite.hpp"

TWE_Lite twelite(4, 3, 38400);

struct Hoge {
	uint8_t u8;
	uint16_t u16;
	char str[5];
}__attribute__((__packed__));

struct Fuga {
	float vec[3];
}__attribute__((__packed__));

void setup(){
	Serial.begin(38400);
	Serial.println("setup");

	twelite.init();

	Hoge hoge;
	hoge.u8 = 0xaa;
	hoge.u16= 0xbeaf;
	hoge.str[0] = 'h';
	hoge.str[1] = 'e';
	hoge.str[2] = 'l';
	hoge.str[3] = 'l';
	hoge.str[4] = 'o';

	Fuga fuga;
	fuga.vec[0] = 1.0 / 1000000000000000;
	fuga.vec[1] = 1234.56;
	fuga.vec[2] = 1.0 / 3;

	twelite.send_simple(0x01, 0x02, hoge);
	twelite.send_simple(0x01, 0x03, fuga);
	while(true);
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
