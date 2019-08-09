#define TWE_LITE_USE_HARDWARE_SERIAL
#include "../TWE-Lite.hpp"

TWE_Lite twe(0, 1, 38400);

#define PIN_LED	13

void setup(){
	twe.init();
	twe.send_buf[0] = 0x01;
	pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, LOW);
}

void loop(){
	if(twe.send(0x78, 1)){
		digitalWrite(PIN_LED, HIGH);
		delay(100);
		digitalWrite(PIN_LED, LOW);
		delay(100);
	}
}
