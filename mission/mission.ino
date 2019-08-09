#include <MsTimer2.h>
#include <Wire.h>				// i2c
#include <Adafruit_BMP280.h>	// BMP280ライブラリ

#define BBM

#ifdef BBM		// BBM試験用パラメータ

	#define ALTITUDE_PARACHUTE		135.0		// 開傘高度
	#define ALTITUDE_LEAFING		120.0		// リーフィング解除高度

#else
	// 本番用パラメータ
	#error please set altitude

#endif

// BBM試験(2019/8/4)にて，BMP280を使って高度を推測し，それに基づいてモードの移行を行うことができることを確認

// ピン設定
namespace pin {
	constexpr size_t flight = 2;		// フライトピン

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

	// BMP280
	volatile float			temperature = 0.0;	// 最新の気温
	volatile float			pressure	= 0.0;	// 最新の気圧
	volatile float			altitude	= 0.0;	// 最新の高度
}

// センサ
namespace sensor {
	Adafruit_BMP280			bmp;	// BMP280
}

// 関数
void init_led(const size_t pin);// LED初期設定
void flightpin_handler();		// フライトピンの割り込みハンドラ(離床判定)
void timer_handler();			// タイマ割り込み関数
const float get_altitude();		// BMP280で高度を取得
void error();					// エラー(内蔵LED点滅)

void setup(){
	global::launch_time = millis();
	global::mode = Mode::standby;
	Serial.begin(9600);

	// LED初期設定
	init_led(pin::led_arduino);
//	init_led(pin::led1);
//	init_led(pin::led2);
//	init_led(pin::led3);

	// i2c
	Wire.begin();
	if(!sensor::bmp.begin())	// BMP280
		error();

	// タイマ割り込み設定
	// i2cはタイマ割り込みを使っているので割り込みハンドラ内でi2cアクセスをするなら多重割り込みを許可しなければならない
	MsTimer2::set(1000 / 100, timer_handler); // 100Hzでタイマ割り込み
	MsTimer2::start();

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
			break;
		case Mode::flight:
			break;
		case Mode::rising:
			if(time >= 6*1000){
				// digitalWrite(pin::led1, HIGH);
				global::mode = Mode::parachute;
				Serial.println("mode launch -> parachute");
			}
			break;
		case Mode::parachute:
			// 開傘判定と開傘
			if(altitude >= ALTITUDE_PARACHUTE){
				// digitalWrite(pin::led2, HIGH);	// 開傘(のつもり)
				global::mode = Mode::leafing;
				Serial.println("mode parachute -> leafing");
			}
			break;
		case Mode::leafing:
			// リーフィング判定とリーフィング
			if(altitude <= ALTITUDE_LEAFING){
				// digitalWrite(pin::led3, HIGH);	// リーフィング解除(のつもり)
				Serial.println("leafing!");
			}
			break;
	}

	Serial.print(global::temperature - 273.15);
	Serial.print(" ");
	Serial.print(global::pressure);
	Serial.print(" ");
	Serial.println(altitude);
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
	global::mode = Mode::flight;	// フライトモードに移行
}

void timer_handler(){
	using namespace sensor;

	// i2cを使うので一時的に割り込み許可
	interrupts();
	global::temperature	= sensor::bmp.readTemperature();	// 気温(C)
	global::pressure	= sensor::bmp.readPressure();		// 気圧(Pa)
	noInterrupts();

	global::temperature	= global::temperature + 273.15;		// Kにする
	global::pressure	= global::pressure / 100.0f;		// hPaにする

	// 高度を計算する
	const auto &t = global::temperature;
	const auto &p = global::pressure;
	global::altitude = (pow(1013.0/p, 1/5.257) - 1)*(t) / 0.0065;
}

void error(){
	while(true){
		digitalWrite(pin::led_arduino, HIGH);
		delay(100);
		digitalWrite(pin::led_arduino, LOW);
		delay(100);
	}
}
