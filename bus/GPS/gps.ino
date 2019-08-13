//#define GPS_USE_HARDWARE_SERIAL
#include "GPS.hpp"

#define BRATE 9600

GPS gps(4, 3, BRATE);

void setup(){
	Serial.begin(BRATE);
	Serial.println("setup");

	gps.init();
	gps.set_interval(1000 / 5);
	gps.send_cmd("314,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");

	delay(1000);
}

void loop(){
//	if(gps.available() > 0){
//		Serial.write(gps.read());
//		Serial.write(" ");
//	}

	if(gps.parse()){
//		Serial.println("success");
	}

	delay(1000 / 5);
}
