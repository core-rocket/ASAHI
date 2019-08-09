#include <MsTimer2.h>
#include <Wire.h>				// i2c
#include <Adafruit_BMP280.h>	// BMP280ライブラリ

// 本番時はこの下の行をコメントアウトすること！！！！
#define BBM

#ifdef BBM		// BBM試験用パラメータ

	#define ALTITUDE_PARACHUTE		135.0
	#define ALTITUDE_LEAFING		120.0


#else
	// 本番用パラメータ
	// 設定には最新の搭載計器安全審査書を参考にすること．
	// また，シミュレーション担当の人間に確認を取ること．

	// 高度の設定(単位は全てm)
	//#define ALTITUDE_PARACHUTE						// 開傘高度(m)
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
	volatile size_t			bmp_count	= 0;				// バッファ書き込み番号
	volatile float			temperature[BMP280_BUF_SIZE];	// 気温(℃ )
	volatile float			pressure[BMP280_BUF_SIZE];		// 気圧(Pa)
	volatile float			altitude	= 0.0;				// 最新の高度(m)
}

// センサ
namespace sensor {
	Adafruit_BMP280			bmp;	// BMP280
}

// 関数
void init_led(const size_t pin);// LED初期設定
void flightpin_handler();		// フライトピンの割り込みハンドラ(離床判定)
void timer_handler();			// タイマ割り込み関数
void update_altitude();			// 高度を更新する
void error();					// エラー(内蔵LED点滅)

void setup(){
	global::launch_time = millis();
	global::mode = Mode::standby;
	Serial.begin(9600);

	// LED初期設定
	init_led(pin::led_arduino);

	// i2c
	Wire.begin();
	if(!sensor::bmp.begin())	// BMP280
		error();

	// タイマ割り込み設定
	MsTimer2::set(1000 / BMP280_SAMPLING_RATE, timer_handler);

	// フライトピン設定
	pinMode(pin::flight, INPUT_PULLUP);

	// 割り込み有効化
	MsTimer2::start();
	attachInterrupt(digitalPinToInterrupt(pin::flight), flightpin_handler, CHANGE);
}

void loop(){
	const auto& launch_time = global::launch_time;
	const auto time = millis() - launch_time;	// 離床からの時間

	update_altitude();							// 高度を更新する
	const auto& altitude = global::altitude;

	switch(global::mode){
		case Mode::standby:
			break;
		case Mode::flight:
			break;
		case Mode::rising:
			if(time >= TIME_RISING){
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

//	Serial.println(altitude);
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
	global::temperature[global::bmp_count]	= bmp.readTemperature();	// 気温(℃ )
	global::pressure[global::bmp_count]		= bmp.readPressure();		// 気圧(Pa)
	noInterrupts();
	global::bmp_count++;
	if(global::bmp_count == BMP280_BUF_SIZE)
		global::bmp_count = 0;
}

void update_altitude(){
	constexpr float p_0 = 1013.0;					// 海面気圧(hPa)

	float t = 0;
	float p = 0;

	// 移動平均を計算する
	for(size_t i=0;i<BMP280_BUF_SIZE;i++){
		t += global::temperature[i];
		p += global::pressure[i];
	}
	t = t / BMP280_BUF_SIZE;
	p = p / BMP280_BUF_SIZE;

	t = t + 273.15;		// Kにする
	p = p / 100.0f;		// hPaにする

	global::altitude = ((pow(p_0/p, 1/5.257) - 1)*t) / 0.0065;
}

void error(){
	while(true){
		digitalWrite(pin::led_arduino, HIGH);
		delay(100);
		digitalWrite(pin::led_arduino, LOW);
		delay(100);
	}
}
