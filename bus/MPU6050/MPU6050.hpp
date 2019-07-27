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
		SELF_TEST_X			= 0x0D, // RW
		SELF_TEST_Y,
		SELF_TEST_Z,
		SELF_TEST_A,
		SMPLRT_DIV			= 0x19, // RW
		CONFIG				= 0x1A, // RW
		GYRO_CONFIG,
		ACCEL_CONFIG,
		FIFO_EN				= 0x23, // RW
		I2C_MST_CTRL		= 0x24, // RW
		I2C_SLV0_ADDR		= 0x25, // RW
		I2C_SLV0_REG,
		I2C_SLV0_CTRL,
		I2C_SLV1_ADDR		= 0x28, // RW
		I2C_SLV1_REG,
		I2C_SLV1_CTRL,
		I2C_SLV2_ADDR		= 0x2B, // RW
		I2C_SLV2_REG,
		I2C_SLV2_CTRL,
		I2C_SLV3_ADDR		= 0x2E, // RW
		I2C_SLV3_REG,
		I2C_SLV3_CTRL,
		I2C_SLV4_ADDR		= 0x31, // RW
		I2C_SLV4_REG,
		I2C_SLV4_DO,
		I2C_SLV4_CTRL,
		I2C_SLV4_DI			= 0x35, // R
		I2C_MST_STATUS		= 0x36, // R
		INT_PIN_CFG			= 0x37, // RW
		INT_ENABLE,
		INT_STATUS			= 0x3A, // R
		ACCEL_XOUT_H		= 0x3B, // R
		ACCEL_XOUT_L,
		ACCEL_YOUT_H,
		ACCEL_YOUT_L,
		ACCEL_ZOUT_H,
		ACCEL_ZOUT_L,
		TEMP_OUT_H,
		TEMP_OUT_L,
		GYRO_XOUT_H,
		GYRO_XOUT_L,
		GYRO_YOUT_H,
		GYRO_YOUT_L,
		GYRO_ZOUT_H,
		GYRO_ZOUT_L,
//		EXT_SENS_DATA_00	= 0x49, // R
//		EXT_SENS_DATA_01,
//		...
//		EXT_SENS_DATA_23,
		EXT_SENS_DATA_START	= 0x49, // R (0~23)
		EXT_SENS_DATA_END	= EXT_SENS_DATA_START + 23,
		I2C_SLV0_DO			= 0x63, // RW
		I2C_SLV1_DO,
		I2C_SLV2_DO,
		I2C_SLV3_DO,
		I2C_MST_DELAY_CTRL	= 0x67, // RW
		SIGNAL_PATH_RESET	= 0x68, // RW
		USER_CTRL			= 0x6A, // RW
		PWR_MGMT_1			= 0x6B, // RW
		PWR_MGMT_2,
		FIFO_COUNT_H		= 0x72, // RW
		FIFO_COUNT_L,
		FIFO_R_W			= 0x74, // RW
		WHO_AM_I			= 0x75, // R
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
	MPU6050() : reg(Reg::WHO_AM_I), error(Error::No), stop(true) {}

	// 初期化
	void init(){
		write8(Reg::WHO_AM_I, 0x00);
		write8(Reg::PWR_MGMT_1, 0x00);
		error = Error::No;
	}

	inline const Error get_error() const {
		return error;
	}

	// データ取得
	const data_t& get_data(){
		begin_reg(Reg::ACCEL_XOUT_H);
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
