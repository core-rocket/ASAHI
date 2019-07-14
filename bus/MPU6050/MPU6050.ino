// MPU6050のテスト用コード
// データシート: https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Components/General%20IC/PS-MPU-6000A.pdf
#include <Wire.h>

#define MPU6050_ADDR			0x68
#define MPU6050_WHO_AM_I		0x75	// R
#define MPU6050_PWR_MGMT_1		0x6b	// R/W
#define MPU6050_ACCEL_XOUT_H	0x3b	// R

void setup(){
	int error;

	Serial.begin(9600);
//	Serial.println("MPU6050");

	Wire.begin();

	Wire.beginTransmission(MPU6050_ADDR);	// 通信開始．
	Wire.write(MPU6050_WHO_AM_I);
	Wire.write(0x00);
	Wire.endTransmission();					// 通信終了．データはこのタイミングで書き込まれる．

	Wire.beginTransmission(MPU6050_ADDR);
	Wire.write(MPU6050_PWR_MGMT_1);
	Wire.write(0x00);
	Wire.endTransmission();

	while(true){
		if(Serial.available() < 0) continue;
		char c = Serial.read();
		if(c == 's') break;
	}
}

void loop(){
	Wire.beginTransmission(MPU6050_ADDR);
	Wire.write(0x3b);
	Wire.endTransmission(false);

	Wire.requestFrom(MPU6050_ADDR, 14, true);
	while(Wire.available() < 14);
	int16_t araw[3], graw[3], temperature;

	araw[0] = Wire.read() << 8 | Wire.read();
	araw[1] = Wire.read() << 8 | Wire.read();
	araw[2] = Wire.read() << 8 | Wire.read();
	temperature = Wire.read() << 8 | Wire.read();
	graw[0] = Wire.read() << 8 | Wire.read();
	graw[1] = Wire.read() << 8 | Wire.read();
	graw[2] = Wire.read() << 8 | Wire.read();

	float a[3], g[3];
	for(int i=0;i<3;i++){
		a[i] = araw[i] / 16384.0;	// PS_SEL_0 16384 LSB / g
		g[i] = graw[i] / 131.0;		// PS_SEL_0 131 LSB / g
	}

	// print data
	for(int i=0;i<3;i++){
		Serial.print(a[i]);
		Serial.print(" ");
	}

	for(int i=0;i<3;i++){
		Serial.print(g[i]);
		Serial.print(" ");
	}
	Serial.println((temperature + 12412.0) / 340.0);
}
