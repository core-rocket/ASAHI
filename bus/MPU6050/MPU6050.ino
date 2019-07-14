// MPU6050のテスト用コード
// データシート: https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Components/General%20IC/PS-MPU-6000A.pdf
#include <Wire.h>

class MPU6050 {
public:
	// 定数
	constexpr static uint8_t addr	= 0x68;	// i2c addr
	enum class Reg : uint8_t {
		accel_xout_h	= 0x3b,	// r
		pwr_mgmt_1		= 0x6b,	// rw
		who_am_i		= 0x75,	// r
	};

	enum class Error : int {
		No	= 0,
		Begin,
		Sync,
		Write,
	};

private:
	Reg reg;		// 現在のレジスタ
	Error error;	// エラー
	bool stop;	// endTransmission時(syncと呼ぶ)にi2cバスを解放するかどうか

public:
	// コンストラクタ
	MPU6050() : reg(Reg::who_am_i), error(Error::No), stop(true) {}

	inline void begin_reg(const Reg &r){
		int ret;
		Wire.beginTransmission(addr);
		ret = Wire.write(static_cast<uint8_t>(r));
		if(ret!=1)
			error = Error::Begin;
	}
	inline void begin_reg(){ begin_reg(reg); }

	inline void sync(const bool s=true){
		int ret = Wire.endTransmission(stop);
		if(ret!=0)
			error = Error::Sync;
	}
	inline void sync(){
		sync(stop);
	}

	inline uint8_t read8(){
		return Wire.read();
	}
	uint8_t read8(const Reg &r){
		begin_reg(r);
		uint8_t data = read8();
		sync(stop);
		return data;
	}

	// signed
	inline int16_t read16(){
		// big endian -> little endian
		return (read8() << 8 | read8());
	}
	int16_t read16(const Reg &r){
		begin_reg(r);
		uint16_t data = read16();
		sync(stop);
		return data;
	}

	inline void write8(const uint8_t &val){
		int ret;
		ret = Wire.write(val);
		if(ret != 1)
			error = Error::Write;
	}
	inline void write8(const Reg &r, const uint8_t &val){
		begin_reg(r);
		write8(val);
		sync(stop);
	}
};

MPU6050 mpu;

void setup(){
	Serial.begin(9600);
	Wire.begin();

	while(true){
		if(Serial.available() < 0) continue;
		if(Serial.read() == 's') break;
	}

	Serial.println("MPU6050");

	mpu.write8(MPU6050::Reg::who_am_i, 0x00);
	mpu.write8(MPU6050::Reg::pwr_mgmt_1, 0x00);
}

void loop(){
	float acc[3], gyro[3], temperature;

	mpu.begin_reg(MPU6050::Reg::accel_xout_h);
	mpu.sync(false);

	Wire.requestFrom(MPU6050::addr, 14, true);
	while(Wire.available() < 14);

	for(int i=0;i<3;i++)
		acc[i] = static_cast<float>(mpu.read16()); // signedなことに注意
	temperature = static_cast<float>(mpu.read16());
	for(int i=0;i<3;i++)
		gyro[i] = static_cast<float>(mpu.read16());
	
	for(int i=0;i<3;i++){
		Serial.print(acc[i] / 16384.0);
		Serial.print(" ");
	}
	for(int i=0;i<3;i++){
		Serial.print(gyro[i] / 131.0);
		Serial.print(" ");
	}
	Serial.println((temperature+12412.0)/340.0);

	delay(500);
}
