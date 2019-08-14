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

void print_float(const GPS::float_t &f){
	Serial.print(f.int_part);
	Serial.print(".");
	Serial.print(f.dec_part);
}

void print_data(const GPS::data_t &d){
	if(!d.valid)
		Serial.print("invalid: ");
	Serial.print("UTC: ");
	print_float(d.time);
	Serial.print("\tlat = ");
	print_float(d.latitude);
	Serial.print("\tlng = ");
	print_float(d.longitude);
	Serial.println("");
}

void loop(){
	if(gps.parse()){
		print_data(gps.data);
	}

	char test[] = "$GPGLL,3539.6473,N,13921.9736,E,092218.600,A,A*56\r\n";
	for(size_t i=0;i<sizeof(test);i++){
//		if(gps.parse8(test[i]))
//			print_data(gps.data);
	}

//	delay(1000 / 5);
}
