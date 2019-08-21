#include<Servo.h>

#define PIN_SERVO 9

Servo servo;

void setup(){
	Serial.begin(9600);
	Serial.println("setup");
	servo.attach(PIN_SERVO);
}

void loop(){
	Serial.println("loop");

	for(size_t i=0;i<100;i++){
		servo.write(i);
		delay(100);
	}
}
