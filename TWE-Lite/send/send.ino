#include <SoftwareSerial.h>

#define BRATE	38400

SoftwareSerial TWE(4, 3); // RX, TX

void setup(){
	Serial.begin(BRATE);
	TWE.begin(BRATE);
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
