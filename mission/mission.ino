//#include <MsTimer2.h>					// タイマー
#include <Wire.h>						// i2c
#include <Adafruit_BMP280.h>			// BMP280ライブラリ
//#include <Servo.h>						// サーボ
#include <VarSpeedServo.h>

#include "../TWE-Lite/TWE-Lite.hpp"		// TWE-Lite
#include "../telemetry.hpp"

// 本番時はこの下の行をコメントアウトすること！！！！
//#define BBM

#ifdef BBM		// BBM試験用パラメータ

	#define ALTITUDE_LEAFING		120.0


#else
	// 本番用パラメータ
	// 設定には最新の搭載計器安全審査書を参考にすること．
	// また，シミュレーション担当の人間に確認を取ること．

	// 高度の設定(単位は全てm)
	#define ALTITUDE_LEAFING		315.0				// リーフィング解除高度(m)
#endif

// 時間の設定(単位は全てミリ秒)
// 離床からの経過時刻を示す)
#define TIME_RISING				(6.0  * 1000)		// 離床後開傘判定を開始する時間

#define TIMEOUT_PARACHUTE		(12.5 * 1000)		// 開傘を強制的に行う時間
#define TIMEOUT_LEAFING			(16.0 * 1000)		// リーフィングを強制的に行う時間

#define BMP280_SAMPLING_RATE	100					// BMP280のサンプリングレート(Hz)
#define BMP280_BUF_SIZE			5					// BMP280のバッファサイズ．移動平均のnでもある

// ピン設定
namespace pin {
	constexpr size_t flight		= 2;		// フライトピン
	constexpr size_t nichrome	= 5;		// ニクロム線
	constexpr size_t servo		= 9;		// サーボ

	// LED
	constexpr size_t led_arduino = 13;
//	constexpr size_t led1 = 3;	// LED1
//	constexpr size_t led2 = 4;	// LED2
//	constexpr size_t led3 = 5;	// LED3
}

// 動作モード
enum class Mode : uint8_t {
	standby,			// スタンバイモード(ロック中)
	flight,				// フライトモード(離床判定)
	rising,				// 上昇モード(離床後6秒間)
	parachute,			// パラシュートモード(開傘判定&開傘)
	leafing,			// リーフィングモード(リーフィング判定&リーフィング)
};

// グローバル変数
namespace global {
	// 割り込みハンドラから読み書きする変数
	volatile Mode			mode;				// 動作モード
	volatile unsigned long	launch_time = 0;	// 離床からの経過時刻
	size_t					descent_count=0;	// 連続下降回数

	// BMP280
	volatile uint32_t	bmp_last_time	= 0;			// 最後にデータ取得した時刻
	volatile size_t		bmp_count		= 0;			// バッファ書き込み番号
	volatile float		temp_buf[BMP280_BUF_SIZE];		// 気温バッファ(℃ )
	volatile float		press_buf[BMP280_BUF_SIZE];		// 気圧バッファ(Pa)
	float				temperature		= 0.0;			// 移動平均をとった気温
	float				pressure		= 0.0;			// 移動平均をとった気圧
	float				last_altitude	= 0.0;			// 1つ前の高度
	float				altitude		= 0.0;			// 最新の高度(m)
}

// センサ
namespace sensor {
	Adafruit_BMP280			bmp;	// BMP280
}

VarSpeedServo		servo;
TWE_Lite	twe(4, 3, 38400);

// 関数
void init_led(const size_t pin);// LED初期設定
void flightpin_handler();		// フライトピンの割り込みハンドラ(離床判定)
void timer_handler();			// タイマ割り込み関数
void update_altitude();			// 高度を更新する
void send_hk();					// HKデータ送信
void send_telemetry();			// テレメトリ送信(toバス部)
void error();					// エラー(内蔵LED点滅)

void setup(){
	global::launch_time = millis();
	global::mode = Mode::standby;
	Serial.begin(38400);

	// LED初期設定
	init_led(pin::led_arduino);

	// i2c
	Wire.begin();
	if(!sensor::bmp.begin())	// BMP280
		error();

	// タイマ割り込み設定
//	MsTimer2::set(1000 / BMP280_SAMPLING_RATE, timer_handler);

	// フライトピン設定
	pinMode(pin::flight, INPUT_PULLUP);

	// ニクロム線
	pinMode(pin::nichrome, OUTPUT);
	digitalWrite(pin::nichrome, LOW);

	// サーボ初期化(ロック時0, 解放時60)
	servo.attach(pin::servo);
	servo.write(0, 100, true);				// なんらかの要因でリセットしても解放しない

	// TWE-Lite初期化
	twe.init();

	// 割り込み有効化
//	MsTimer2::start();
	attachInterrupt(digitalPinToInterrupt(pin::flight), flightpin_handler, CHANGE);
}

void timerrrrrrrrrr(){
	static uint32_t last = 0;
	const uint32_t now = millis();
	if((now - last) > (1000 / BMP280_SAMPLING_RATE)){
		timer_handler();
		last = now;
	}
}

void loop(){
	const auto& launch_time = global::launch_time;
	const auto time = millis() - launch_time;	// 離床からの時間

	update_altitude();							// 高度を更新する
	const auto& last_altitude = global::last_altitude;
	const auto& altitude = global::altitude;

	switch(global::mode){
		case Mode::standby:
			Serial.println("mode: standby");
			delay(300);
			if(twe.try_recv(100)){
				Serial.println("recv");
				if(twe.from_id() != id_station)
					break;
				if(twe.is_extended()){
					// コマンド
					//if(twe.response_id() == 0x04){		// フライトモード移行
					//	global::mode = Mode::flight;
					//	Serial.println("flight mode on");
					//	servo.write(0, 100, true);
					//}

					switch(twe.response_id()){
						case 0x02:				// 縦解放機構ロック解除
							servo.write(60, 100, true);
							break;
						case 0x03:				// 縦解放機構ロック
							servo.write(0, 100, true);
							break;
						case 0x04:				// フライトモードON
							global::mode = Mode::flight;
							break;
					}
				}
			}
			break;
		case Mode::flight:
			Serial.println("mode: flight");
			break;
		case Mode::rising:
			Serial.println("mode: rising");
			if(time >= TIME_RISING){
				// digitalWrite(pin::led1, HIGH);
				global::mode = Mode::parachute;
				Serial.println("mode launch -> parachute");
			}
			break;
		case Mode::parachute:
			// 開傘判定と開傘
			Serial.println("mode: parachute");

			// 下降しているかどうか
			if(static_cast<int>(altitude*10) < static_cast<int>(last_altitude*10))
				global::descent_count++;
			else
				global::descent_count = 0;

			// 開傘判定
			// 5回連続で下降 or タイムアウト
			if(global::descent_count >= 4 || time > TIMEOUT_PARACHUTE){
				Serial.println("do parachute!!!!");

				servo.write(60, 100, true);

				global::mode = Mode::leafing;
				Serial.println("mode parachute -> leafing");
			}

			break;
		case Mode::leafing:
			// リーフィング判定とリーフィング
			Serial.println("mode: leafing");

			// リーフィング判定
			// 指定高度以下になったら or タイムアウト
			if(altitude <= ALTITUDE_LEAFING || time > TIMEOUT_LEAFING){
				digitalWrite(pin::nichrome, HIGH);
				Serial.println("leafing!");
			}
			break;
	}

	timerrrrrrrrrr();

	send_hk();
	send_telemetry();

	Serial.print("launch=");
	Serial.print(global::launch_time);
	Serial.print(", altitude=");
	Serial.print(altitude);
	Serial.print(", descent=");
	Serial.println(global::descent_count);

//	delay(100);
}

void init_led(const size_t pin){
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

void flightpin_handler(){
	const auto t = millis();
	if(global::mode != Mode::flight){		// フライトモード以外でフライトピンに変化があった
		return;		// とりあえずなにもしない
	}

	// TODO: チャタリング検知?
	detachInterrupt(digitalPinToInterrupt(pin::flight));	// ピン割り込みを解除

	global::launch_time = t;		// 離床時刻
	global::mode = Mode::rising;	// 飛翔モードに移行
}

void timer_handler(){
	using namespace sensor;

	global::bmp_last_time = millis();

	// i2cを使うので一時的に割り込み許可
//	interrupts();
	global::temp_buf[global::bmp_count]		= bmp.readTemperature();	// 気温(℃ )
	global::press_buf[global::bmp_count]	= bmp.readPressure();		// 気圧(Pa)
//	noInterrupts();
	global::bmp_count++;
	if(global::bmp_count == BMP280_BUF_SIZE)
		global::bmp_count = 0;
}

void update_altitude(){
	constexpr float p_0 = 1013.0;					// 海面気圧(hPa)

	float &t = global::temperature;
	float &p = global::pressure;

	t = 0.0;
	p = 0.0;

	// 移動平均を計算する
	for(size_t i=0;i<BMP280_BUF_SIZE;i++){
		t += global::temp_buf[i];
		p += global::press_buf[i];
	}
	t = t / BMP280_BUF_SIZE;
	p = p / BMP280_BUF_SIZE;

	t = t + 273.15;		// Kにする
	p = p / 100.0f;		// hPaにする

	global::last_altitude = global::altitude;
	global::altitude = ((pow(p_0/p, 1/5.257) - 1)*t) / 0.0065;
}

void send_hk(){
	static uint32_t last = 0;
	uint32_t now = millis();
	// ミッション部のシーケンス状況送信
	if((now - last) > 100){
		twe.send_extend(id_station, 0x01, static_cast<uint8_t>(global::mode));
		last = now;
	}
}

void send_telemetry(){
	Float32 data;
	data.time = global::bmp_last_time;

	// 気温(K)
	data.value = global::temperature;
	twe.send_simple(id_station, 0x04, data);
	delay(10);

	// 気圧(hPa)
	data.value = global::pressure;
	twe.send_simple(id_station, 0x05, data);
	delay(10);

	// 高度(m)
	data.value = global::altitude;
	twe.send_simple(id_station, 0x06, data);
	delay(10);
}

void error(){
	Serial.println("error");
//	while(true){
//		digitalWrite(pin::led_arduino, HIGH);
//		delay(100);
//		digitalWrite(pin::led_arduino, LOW);
//		delay(100);
//	}
}
