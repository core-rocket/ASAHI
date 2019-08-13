#define GPS_USE_HARDWARE_SERIAL
#include "GPS.hpp"

#define BRATE 38400

GPS gps(BRATE);

void setup(){
	delay(5000);

	gps.init();
	gps.set_interval(1000 / 10);

	delay(1000);
}

void loop(){
	if(gps.available() > 0){
		Serial.write(gps.read());
		Serial.write(" ");
	}
}
