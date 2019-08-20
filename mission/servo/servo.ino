#include<Servo.h>

#define PIN_SERVO 9

Servo servo;

void setup(){
	servo.attach(PIN_SERVO);
	servo.write(0);
	delay(1000);
	servo.write(75);
}

void loop(){
	servo.write(0);
	delay(1000);
	servo.write(75);
	delay(1000);
}
