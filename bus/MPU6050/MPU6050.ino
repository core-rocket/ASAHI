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

	struct data_t {
		int16_t acc[3];
		int16_t temperature;
		int16_t gyro[3];
	} data;
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

	void init(){
		write8(Reg::who_am_i, 0x00);
		write8(Reg::pwr_mgmt_1, 0x00);
	}

	const data_t& get_data(){
		begin_reg(Reg::accel_xout_h);
		sync(false);
		Wire.requestFrom(addr, 14, true);
		while(Wire.available() < 14);	// データが貯まるまで待つ
		for(int i=0;i<3;i++)
			data.acc[i] = read16();
		data.temperature= read16();
		for(int i=0;i<3;i++)
			data.gyro[i]= read16();
		return data;
	}
};

MPU6050 mpu;

void setup(){
	Serial.begin(9600);
	Wire.begin();

	// 's'が入力されるまで待つ
	while(true){
		if(Serial.available() < 0) continue;
		if(Serial.read() == 's') break;
	}

	Serial.println("MPU6050");
	mpu.init();
}

void loop(){
	auto data = mpu.get_data();

	for(int i=0;i<3;i++){
		Serial.print(static_cast<float>(data.acc[i]) / 16384.0);
		Serial.print(" ");
	}
	for(int i=0;i<3;i++){
		Serial.print(static_cast<float>(data.gyro[i]) / 131.0);
		Serial.print(" ");
	}
	Serial.println((static_cast<float>(data.temperature)+12412.0)/340.0);

	delay(500);
}
