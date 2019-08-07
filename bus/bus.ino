#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.h"
#define GPS_USE_HARDWARE_SERIAL
#include "GPS/GPS.hpp"
#include "MPU6050/MPU6050.hpp"

// ボードレート
#define BRATE	38400

// 動作モード
enum class Mode : char {
	Wait,		// ウェイトモード．コマンド受領までなにもしないをする．
	Standby,	// スタンバイモード．離床判定を行う．
	Flight,		// フライトモード．離床〜開傘まで．
	Descent,	// ディセントモード．開傘〜着水まで．
};

// グローバル変数
Mode g_mode;
MPU6050 mpu;
GPS gps(BRATE);
TWE_Lite twelite(6, 5, BRATE);

// 初期化関数．一度だけ実行される．
void setup(){
	//TODO: センサ初期化
	Wire.begin();
	mpu.init();
	gps.init();
	delay(1000);

	//TODO: TWE-Lite初期化
	twelite.init();

	//TODO: 動作モードをSDカードから読み込む
	// (動作中に瞬断して再起動する可能性がある)
	if(g_mode != Mode::Standby)
		return;		// 再起動時は早くそのモードの動作に戻る
	//TODO: 地上局に起動を通知
}

void loop(){
	switch(g_mode){
		case Mode::Wait:
			break;
		case Mode::Standby:
			break;
		case Mode::Flight:
			break;
		case Mode::Descent:
			break;
	}

	auto motion = mpu.get_data();
	Serial.print("acc[0] = ");
	Serial.println(static_cast<float>(motion.acc[0]) / 16384.0);

	Serial.print("GPS: ");
	for(size_t i=0;i<500;i++){
		const int c = gps.read();
		if(c >= 0)
			Serial.write((char)c);
	}
	Serial.println("");

	// テレメトリ送信
	twelite.send_simple(0x01, 0x00, "send string test");

	// 加速度
	Vec16_t telem_acc;
	telem_acc.x = motion.acc[0];
	telem_acc.y = motion.acc[1];
	telem_acc.z = motion.acc[2];
	twelite.send_simple(0x01, 0x01, telem_acc);

	// 角速度
	Vec16_t telem_gyro;
	telem_gyro.x= motion.gyro[0];
	telem_gyro.y= motion.gyro[1];
	telem_gyro.z= motion.gyro[2];
	twelite.send_simple(0x01, 0x02, telem_gyro);

	if(twelite.check_send() == 1){
		Serial.println("TWE-Lite send success");
	}else{
		Serial.println("TWE-Lite send failed");
	}

	delay(300);
}
