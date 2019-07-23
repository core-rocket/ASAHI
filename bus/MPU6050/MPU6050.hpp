#ifndef MPU6050_HPP_
#define MPU6050_HPP_

#include <Wire.h>

// MPU6050のヘッダオンリライブラリ
// データシート: https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Components/General%20IC/PS-MPU-6000A.pdf
//
// ヘッダオンリである必要はあまり無い気がするが，
// Arduinoで使うわけだし，同じプロジェクト内で複数の.ino/.cppからincludeされることとかも無さそう
// 将来的にはi2cのクラスを作ってそこから派生して各センサのクラスをそれぞれのヘッダとして生やしていくとかしたい

class MPU6050 {
public:
	// 定数
	constexpr static uint8_t i2c_addr = 0x68;	// i2c addr

	// registers
	enum class Reg : uint8_t {
		accel_xout_h	= 0x3b, // r
		pwr_mgmt_1		= 0x6b, // rw
		who_am_i		= 0x75, // r
	};

	// error type
	enum class Error : uint8_t {
		No	= 0,
		Begin,
		Sync,
		Write,
	};

	// data struct
	union data_t {
		uint16_t raw[3 + 1 + 3];
		struct {
			int16_t acc[3]; // signed
			int16_t temperature;
			int16_t gyro[3];
		};
	};

private:
	Reg		reg;	// 現在選択しているレジスタ
	Error	error;
	data_t	data;
	bool	stop;	// endTransmission(=sync)時にi2cバスを解放するかどうか

public:
	// コンストラクタ
	MPU6050() : reg(Reg::who_am_i), error(Error::No), stop(true) {}

	// 初期化
	void init(){
		write8(Reg::who_am_i, 0x00);
		write8(Reg::pwr_mgmt_1, 0x00);
		error = Error::No;
	}

	inline const Error get_error() const {
		return error;
	}

	// データ取得
	const data_t& get_data(){
		begin_reg(Reg::accel_xout_h);
		sync(false);
		Wire.requestFrom(i2c_addr, 14, true); // TODO: 抽象化
		for(int i=0;i<3+1+3;i++)
			data.raw[i] = static_cast<int16_t>(read16_raw());
		return data;
	}

	// 以下の関数はi2cへの実際のアクセスをする関数群
	inline void begin_reg(const Reg &reg){
		this->reg = reg;
		Wire.beginTransmission(i2c_addr);
		int ret = Wire.write(static_cast<uint8_t>(reg));
		if(ret != 1)
			error = Error::Begin;
	}

	inline void sync(const bool s=true){
		int ret = Wire.endTransmission(stop);
		if(ret != 0)
			error = Error::Sync;
	}

	inline const uint8_t read8_raw() const {
		return Wire.read();
	}

	inline const uint8_t read8(const Reg &r){
		begin_reg(r);
		auto r8 = read8_raw();
		sync();
		return r8;
	}

	inline const uint16_t read16_raw() const { // begin_regを使わない場合
		// big endian -> little endian
		return (read8_raw() << 8 | read8_raw());
	}

	inline const uint16_t read16(const Reg &r){
		begin_reg(r);
		auto r16 = read16_raw();
		sync();
		return r16;
	}

	inline void write8_raw(const uint8_t &val){
		int ret = Wire.write(val);
		if(ret != 1)
			error = Error::Write;
	}

	inline void write8(const Reg &r, const uint8_t &val){
		begin_reg(r);
		write8_raw(val);
		sync();
	}
};

#endif
