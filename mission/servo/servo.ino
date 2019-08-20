#include<Servo.h>

#define PIN_SERVO 9

Servo servo;

void setup(){
	servo.attach(PIN_SERVO);
	delay(5000);
	servo.write(0);
	delay(5000);
	servo.write(75);
}

void loop(){
}
