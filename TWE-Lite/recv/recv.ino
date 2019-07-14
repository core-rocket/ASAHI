#include <SoftwareSerial.h>

SoftwareSerial TWE(4, 3); // RX, TX

void setup(){
	Serial.begin(38400);
	TWE.begin(38400);
}

void loop(){
	uint8_t buf[60];
	int count = 0;

	Serial.print("recv: ");
	while(TWE.available()){
		buf[count] = TWE.read();
		count++;
	}

	if(count > 0){
		for(int i=0;i<count;i++)
			Serial.print((char)buf[i]);
	}
	Serial.println("");

	delay(100);
}
