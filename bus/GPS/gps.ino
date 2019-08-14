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

//	if(gps.parse()){
//		Serial.println("success");
//	}


	while(gps.available() != 0){
		int c = gps.read();
		if(c < 0) break;
		//Serial.write((char)c);
		//Serial.write(' ');
		gps.parse8((char)c);
	}


	char test[] = "$GPGLL,3539.6473,N,13921.9736,E,092218.600,A,A*56\r\n";
	for(size_t i=0;i<sizeof(test);i++){
		gps.parse8(test[i]);
	}
//	while(true);

	delay(1000 / 5);
}
