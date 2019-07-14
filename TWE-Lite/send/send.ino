#include <SoftwareSerial.h>

SoftwareSerial TWE(4, 3); // RX, TX

void setup(){
	Serial.begin(38400);
	TWE.begin(38400);
}

void loop(){
	uint8_t data[10] = {':', '7', '8', '0', '1', '3', '5', 'X', '\r', '\n'};

	Serial.print("sending");
	for(int i=0;i<10;i++){
		Serial.print(".");
		TWE.write(data[i]);
	}
	Serial.println("done");
	delay(300);
}
