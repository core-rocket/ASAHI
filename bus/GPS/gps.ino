#define GPS_USE_HARDWARE_SERIAL
#include "GPS.hpp"

#define BRATE 38400

GPS gps(BRATE);

void setup(){
	delay(5000);
//	gps_init.begin(9600);
//	gps_init.write("$PMTK251,38400*27\r\n");
//	gps_init.end();
//	delay(5000);

//	Serial.begin(BRATE);
//	gps.begin(BRATE);
//	gps.end();
//	gps.listen();
//	while(Serial.read() != 's');
//	Serial.println("setup");
//	gps.init(BRATE);

	gps.init();

	delay(1000);
}

void loop(){
	if(gps.available() > 0)
		Serial.write(gps.read());
}
