#include "GPS.hpp"

#define BRATE 38400

GPS gps(4, 5);

void setup(){
	Serial.begin(BRATE);
	while(Serial.read() != 's');
	gps.init(BRATE);
}

void loop(){
	int c = gps.read();
	if(c >= 0)
		Serial.write((char)c);
}
