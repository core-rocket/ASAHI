#include <SD.h>

#define SD_CS_PIN	10

void setup(){
	Serial.begin(38400);
	if(!SD.begin(SD_CS_PIN)){
		Serial.println("SD begin error!");
	}
	Serial.println("SD begin ok");
}

void loop(){

}
