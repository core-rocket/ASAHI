#include <MsTimer2.h>
#include <Wire.h>				// i2c
#include <Adafruit_BMP280.h>	// BMP280ライブラリ

// BBM試験用高度
#define ALTITUDE_PARACHUTE		135.0		// 開傘高度
#define ALTITUDE_LEAFING		120.0		// リーフィング解除高度

// BBM試験(2019/8/4)にて，BMP280を使って高度を推測し，それに基づいてモードの移行を行うことができることを確認

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
	// i2cはタイマ割り込みを使っているので割り込みハンドラ内でi2cアクセスをするなら多重割り込みを許可しなければならない
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
