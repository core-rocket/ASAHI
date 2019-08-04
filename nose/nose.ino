// BBM試験用プログラム(BBMやってないしどうせブレッドボードとかでやるだろうからBBMと呼びます)
// 試験結果が良好であればmasterにマージすること

// 試験内容: 離床判定(フライトピン動作確認), 開傘判定(高度), リーフィング判定(高度)

// 留意点:
// - BMP280は@sk2satはまだ実物を見ていないので，動作報告のある https://gist.github.com/uc-kd/38b5d8bfaaa592cda404996b461a589b を参考に書いた
// - フライトピンはArduino nanoのD2に接続し，ピンが抜ける前後で電圧の変化が発生するようにして下さい(変化する方向は問わない)
// - BMP280のライブラリは https://github.com/adafruit/Adafruit_BMP280_Library からダウンロードすること
// - MsTimer2は https://github.com/PaulStoffregen/MsTimer2 からダウンロードすること
// - LEDのピン番号は内蔵のもの(エラー用)を除き0になっているので適宜設定すること
// - 試験に使用する高度(開傘, リーフィング解除)は適宜設定すること

#include <MsTimer2.h>
#include <Wire.h>				// i2c
#include <Adafruit_BMP280.h>	// BMP280ライブラリ

#define ALTITUDE_PARACHUTE		135.0		// 開傘高度
#define ALTITUDE_LEAFING		120.0		// リーフィング解除高度

// ピン設定
namespace pin {
	constexpr size_t flight = 2;
	constexpr size_t led_arduino = 13;
	// LEDピン番号(適宜書き換えて下さい)
	constexpr size_t led1 = 3;	// LED1
	constexpr size_t led2 = 4;	// LED2
	constexpr size_t led3 = 5;	// LED3
}

// 動作モード
enum class Mode {
	standby,
	launch,
	parachute,
	leafing,
};

// グローバル変数
namespace global {
	// 割り込みハンドラから弄る変数
	volatile unsigned long	launch_time = 0;
	volatile Mode			mode;
	volatile float			altitude = 0.0;

	// デバイス
	Adafruit_BMP280			bmp;
}

// 関数
void init_led(const size_t pin);// LED初期設定
void flightpin_handler();		// フライトピンの割り込みハンドラ(離床判定)
void timer_handler();			// タイマ割り込み関数
const float get_altitude();		// BMP280で高度を取得
void error();					// エラー(内蔵LED点滅)

void setup(){
	global::launch_time = millis();
	global::mode = Mode::launch;
	Serial.begin(9600);

	// LED初期設定
	init_led(pin::led_arduino);
	init_led(pin::led1);
	init_led(pin::led2);
	init_led(pin::led3);

	// i2c
	Wire.begin();
	if(!global::bmp.begin())	// BMP280
		error();

	// タイマ割り込み設定
	MsTimer2::set(1000 / 100, timer_handler); // 100Hzでタイマ割り込み
//	MsTimer2::start();

	// フライトピン割り込み設定
	pinMode(pin::flight, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(pin::flight), flightpin_handler, CHANGE);
}

void loop(){
	const auto& launch_time = global::launch_time;
	const auto time = millis() - launch_time; // 離床からの時間
	const auto& altitude = global::altitude;

	switch(global::mode){
		case Mode::standby:
			// 打ち上げまでにやることがあればやる
			break;
		case Mode::launch:
			if(time >= 6*1000){
				digitalWrite(pin::led1, HIGH);
				global::mode = Mode::parachute;
				Serial.println("mode launch -> parachute");
			}
			break;
		case Mode::parachute:
			// 開傘判定と開傘
			if(altitude >= ALTITUDE_PARACHUTE){
				digitalWrite(pin::led2, HIGH);	// 開傘(のつもり)
				global::mode = Mode::leafing;
				Serial.println("mode parachute -> leafing");
			}
			break;
		case Mode::leafing:
			// リーフィング判定
			if(altitude <= ALTITUDE_LEAFING){
				digitalWrite(pin::led3, HIGH);	// リーフィング解除(のつもり)
				Serial.println("leafing!");
			}
			break;
	}
	global::altitude = get_altitude();
//	Serial.println(global::altitude);
//	delay(100);
}

void init_led(const size_t pin){
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

void flightpin_handler(){
	global::launch_time = millis();
	global::mode = Mode::launch;
	detachInterrupt(digitalPinToInterrupt(pin::flight));
}

void timer_handler(){
	// 高度を更新
	global::altitude = get_altitude();
}

const float get_altitude(){
	constexpr float p_0 = 1013.0;	// 大気圧(hPa)
	const float t = global::bmp.readTemperature();		// C
	const float p = global::bmp.readPressure() / 100.0f;// hPa
	const float a = (pow(p_0/p, 1/5.257) - 1)*(t+273.15) / 0.0065;
	return a;
}

void error(){
	while(true){
		digitalWrite(pin::led_arduino, HIGH);
		delay(100);
		digitalWrite(pin::led_arduino, LOW);
		delay(100);
	}
}
