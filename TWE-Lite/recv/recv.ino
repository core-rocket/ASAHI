#include <SoftwareSerial.h>
#include "../TWE-Lite.hpp"

TWE_Lite twe(4, 3, 38400);

//SoftwareSerial TWE(4, 3); // RX, TX

void setup(){
	Serial.begin(38400);
	twe.init();
//	TWE.begin(38400);
	Serial.println("setup");
}

void loop(){
//	while(true){
//		int b = TWE.read();
//		if(b > 0)
//		Serial.print((char)b, HEX);
//	}

	twe.send_simple(0x00, 0x01, "aaa");
	while(twe.check_send()!=1);
	Serial.println("send: ");
	delay(100);

	size_t size = twe.recv();
	if(size == 0) return;
	Serial.print("recv: ");

	for(size_t i=0;i<size;i++)
		Serial.print(twe.recv_buf[i], HEX);
	Serial.println("");

	delay(100);
}
