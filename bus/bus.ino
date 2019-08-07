#include <MsTimer2.h>				// タイマ
#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.h"
#define GPS_USE_HARDWARE_SERIAL
#include "GPS/GPS.hpp"
#include "MPU6050/MPU6050.hpp"

// ボードレート
#define BRATE		38400

// タイマー関数実行間隔(Hz)
#define TIMER_HZ	100

// センサ
GPS gps(BRATE); // baud変更があるので他のSerialより先に初期化するべき
MPU6050 mpu;

// 無線機
TWE_Lite twelite(6, 5, BRATE);

// センサデータ
namespace sensor_data {
	volatile MPU6050::data_t motion;
}

// 関数
void send_telemetry();	// テレメトリ送信
void timer_handler();	// タイマ割り込みハンドラ

// 文字列でログを送る(あとで消す)
void send_log(const char *str){
	twelite.send_simple(0x01, 0x00, str);
}

// 初期化関数．起動時, リセット時に実行される．
void setup(){
	// GPS初期化
	gps.init();		// baud変更があるので初めに初期化
	delay(1000);	// 念の為少し待つ

	// 無線機初期化
	twelite.init();
	send_log("setup");

	// センサ初期化
	send_log("sensor init");
	Wire.begin();
	mpu.init();
	MsTimer2::set(1000 / TIMER_HZ, timer_handler);
	send_log("finish");

	//TODO: 動作モードをSDカードから読み込む
	// (動作中に瞬断して再起動する可能性がある)
//	if(g_mode != Mode::Standby)
//		return;		// 再起動時は早くそのモードの動作に戻る
	//TODO: 地上局に起動を通知

	// タイマスタート
	MsTimer2::start();
}

// メインループ
void loop(){
//	auto motion = mpu.get_data();

	Serial.print("GPS: ");
	for(size_t i=0;i<gps.available();i++){
		const int c = gps.read();
		if(c >= 0)
			Serial.write((char)c);
	}
	Serial.println("");

	// テレメトリ送信
	send_telemetry();
}

void send_telemetry(){
	using namespace sensor_data;
	const Vec16_t acc = {
		motion.acc[0],
		motion.acc[1],
		motion.acc[2],
	};
	const Vec16_t gyro = {
		motion.gyro[0],
		motion.gyro[1],
		motion.gyro[2],
	};

	twelite.send_simple(0x01, 0x01, acc);
	twelite.send_simple(0x01, 0x02, gyro);
}

void timer_handler(){
	interrupts();	// 割り込み許可
	auto motion = mpu.get_data();
	noInterrupts();	// 割り込み禁止
	sensor_data::motion = motion;
}
