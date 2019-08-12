#include <MsTimer2.h>				// タイマ
#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.hpp"
#define GPS_USE_HARDWARE_SERIAL
#include "GPS/GPS.hpp"
#include "MPU6050/MPU6050.hpp"
#include "queue.hpp"

// ボードレート
#define BRATE		38400

// タイマー関数実行間隔(Hz)
#define TIMER_HZ	10

// センサ
GPS gps(BRATE); // baud変更があるので他のSerialより先に初期化するべき
MPU6050 mpu;

// 無線機
TWE_Lite twelite(4, 3, BRATE);

enum class Mode {
	standby,
	flight,
};

// グローバル変数
namespace global {
	size_t loop_count = 0;				// 何回目のloopか
	unsigned long loop_time = 0;		// 1回のloopにかかった時間
	unsigned long last_loop_time = 0;

	Mode mode;
}

// センサデータ
namespace sensor_data {
	volatile queue<MPU6050::data_t, 10> motion;
	volatile queue<unsigned long, 10> motion_time;
}

// 関数
void send_telemetry();	// テレメトリ送信
void timer_handler();	// タイマ割り込みハンドラ

// 文字列でログを送る(あとで消す)
void send_log(const char *str){
	twelite.send_simple(id_station, 0x00, str);
	Serial.println(str);
	delay(100);
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

	global::mode = Mode::standby;

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
	const uint32_t now = millis();
	global::loop_time = now - global::last_loop_time;
	global::last_loop_time = now;

	global::loop_count++;

//	Serial.print(global::loop_count - 1);
//	Serial.print(" ");
	Serial.println(global::loop_time);

//	Serial.print("GPS: ");
//	for(size_t i=0;i<gps.available();i++){
//		const int c = gps.read();
//		if(c >= 0)
//			Serial.write((char)c);
//	}
//	Serial.println("");

	// 受信
	if(twelite.try_recv(10)){
		Serial.print("recv: ");
		send_log("recv");

		if(twelite.is_response())
			Serial.println("response");

		if(twelite.is_simple()){
			Serial.println("simple");
		}else{
			Serial.println("extend");
			if(twelite.response_id() == 0x02){
				global::mode = Mode::flight;
				send_log("flight mode on");
			}
		}
	}

	// ミッション部へのコマンド送信テスト
	if(global::mode == Mode::flight){
		twelite.send_extend(id_mission, 0x02, " ");
		Serial.println("send flight mode command");
	}

	// テレメトリ送信
	send_telemetry();
//	delay(100);
}

void send_telemetry(){
	using namespace sensor_data;

	Serial.println(motion.size());

	for(size_t i=0;i<motion.size();i++){
		const auto &m = motion.front();
		const auto &t = motion_time.front();
		const Vec16_t acc = {
			t,
			m.acc[0],
			m.acc[1],
			m.acc[2],
		};
		const Vec16_t gyro= {
			t,
			m.gyro[0],
			m.gyro[1],
			m.gyro[2],
		};

		twelite.send_simple(id_station, 0x01, acc);
		twelite.send_simple(id_station, 0x02, gyro);

		motion.pop();
		motion_time.pop();
	}

}

void timer_handler(){
	using namespace sensor_data;
	static size_t count = 0;

	const auto t = millis();

	interrupts();	// 割り込み許可
	auto m = mpu.get_data();
	noInterrupts();	// 割り込み禁止

	if(count % 5 == 0){
		motion_time.push(t);
		motion.push(m);
	}

	count++;
}
