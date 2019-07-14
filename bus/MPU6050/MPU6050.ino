// MPU6050のテスト用コード
// データシート: https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Components/General%20IC/PS-MPU-6000A.pdf
#include <Wire.h>

#define MPU6050_ADDR			0x68
#define MPU6050_WHO_AM_I		0x75	// R
#define MPU6050_PWR_MGMT_1		0x6b	// R/W
#define MPU6050_ACCEL_XOUT_H	0x3b	// R

namespace MPU6050 {
	constexpr uint8_t addr			= 0x68;	// I2C addr
	constexpr uint8_t who_am_i		= 0x75;	// R
	constexpr uint8_t pwr_mgmt_1	= 0x6b;	// R/W
	constexpr uint8_t accel_xout_h	= 0x3b;	// R

int read(const uint8_t &reg, uint8_t *buf, const int &size){
	int i, n, error;
	Wire.beginTransmission(addr);
	n = Wire.write(reg);
	if(n != 1)
		return -10;
	n = Wire.endTransmission(false);	// falseでコネクションを維持
	if(n != 0)
		return n;
	Wire.requestFrom(addr, size, true);	// trueでI2Cバスをread後にrelease
	i = 0;
	for(i=0; Wire.available() && i<size; i++){
		buf[i] = Wire.read();
	}
	if(i != size)
		return 11;

	return 0; // no error
}

int write(const uint8_t &reg, const uint8_t *buf, const size_t &size){
	int n, error;
	Wire.beginTransmission(addr);
	n = Wire.write(reg);
	if(n != 1)
		return -20;
	n = Wire.write(buf, size);
	if(n != size)
		return -21;
	error = Wire.endTransmission(true); // release i2c
	if(error != 0)
		return error;

	return 0; // no error
}

int write_reg(const uint8_t &reg, const uint8_t &data){
	int error;
	error = write(reg, &data, 1);
	return error;
}

} // namespace MPU6050

void setup(){
	int error;
	uint8_t c;

	Serial.begin(9600);
	while(true){
		if(Serial.available() < 0) continue;
		if(Serial.read() == 's') break;
	}

	Serial.println("MPU6050");

	Wire.begin();

	// 初回読み出し
	error = MPU6050::read(MPU6050::who_am_i, &c, 1);
	Serial.print("who_am_i=");
	Serial.print(c);
	Serial.print(", error=");
	Serial.println(error);

	// 動作モード読み出し
	error = MPU6050::read(MPU6050::pwr_mgmt_1, &c, 1);
	Serial.print("pwr_mgmt_1=");
	Serial.print(c);
	Serial.print(", error=");
	Serial.println(error);

	// 動作開始
	MPU6050::write_reg(MPU6050::pwr_mgmt_1, 0);
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
