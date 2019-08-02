#ifndef TWE_LITE_H_
#define TWE_LITE_H_

#ifdef RASPBERRY_PI
	#include <string>
	#include <wiringPi.h>
	#include <wiringSerial.h>
#elif ARDUINO
	#include <SoftwareSerial.h>
#else
	#error please define RASPBERRY_PI or ARDUINO
#endif

class TWE_Lite {
public:
	// コンパイル時定数
	constexpr static long int default_brate	= 115200;
	constexpr static uint8_t buf_size		= 80;
	constexpr static uint16_t MSB			= 0x8000;

	// コンストラクタ
	explicit TWE_Lite(const uint8_t rx, const uint8_t tx, const long int &brate) : rx(rx), tx(tx), brate(brate) {}
	explicit TWE_Lite(const uint8_t rx, const uint8_t tx) : rx(rx), tx(tx), brate(default_brate) {}

	// 定数
	const uint8_t rx, tx;
	const long int brate;

	// publicな変数
	uint8_t send_buf[buf_size];
	uint8_t recv_buf[buf_size];

	// 初期化
	bool init(){
		#ifdef RASPBERRY_PI
		fd = serialOpen(devfile.c_str(), brate);
		if(fd < 0) return false;
		#elif ARDUINO
		serial = new SoftwareSerial(rx, tx);
		serial->begin(brate);
		#endif

		return true;
	}

	// シリアル通信の抽象化
	inline void swrite8(const uint8_t &val){
		#ifdef ARDUINO
		serial->write(val);
		#endif
	}
	inline void swrite16_big(const uint16_t &val){
		swrite8(static_cast<uint8_t>(val >> 8));
		swrite8(static_cast<uint8_t>(val & 0xff));
	}
	inline void swrite(const uint8_t *buf, const size_t &size){
		#ifdef ARDUINO
		serial->write(buf, size);
		#endif
	}

	// 現在のバッファを送信
	bool send(const uint8_t &size){
		uint8_t header[] = { 0xA5, 0x5A }; // binary mode
		uint8_t id = 0x78;
		uint8_t cmd_type = 0x01;

		// 送信コマンド長
		uint16_t cmd_size = MSB + static_cast<uint16_t>(size) + 2; // 簡易形式

		// checksum
		uint8_t checksum = id ^ cmd_type;
		for(uint8_t i=0;i<size;i++)
			checksum = checksum ^ send_buf[i];

		// 送信
		swrite(header, 2);
		swrite16_big(cmd_size);
		swrite8(id);
		swrite8(cmd_type);
		swrite(send_buf, size);
		swrite8(checksum);

		return true;
	}
	// 受信してバッファに突っ込む
	size_t recv(const size_t timeout=0);

private:
#ifdef ARDUINO
	SoftwareSerial *serial = nullptr;
#endif
};

#endif
