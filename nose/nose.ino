// ピン設定
namespace pin {
	constexpr size_t flight = 2;
	constexpr size_t led_arduino = 13;
}

enum class Mode {
	standby,
	launch,
	parachute,
	leafing,
};

// グローバル変数
namespace global {
	volatile unsigned long launch_time = 0;
	volatile Mode mode;
}

void flightpin_handler();

void setup(){
	global::mode = Mode::standby;
	Serial.begin(9600);

	// LED初期設定
	pinMode(pin::led_arduino, OUTPUT);
	digitalWrite(pin::led_arduino, LOW);

	// フライトピン割り込み設定
	pinMode(pin::flight, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(pin::flight), flightpin_handler, CHANGE);
}

void flightpin_handler(){
	global::launch_time = millis();
	global::mode = Mode::launch;
	detachInterrupt(digitalPinToInterrupt(2));
}

void loop(){
	const auto& launch_time = global::launch_time;
	const auto time = millis() - launch_time; // 離床からの時間

	switch(global::mode){
		case Mode::standby:
			// 打ち上げまでにやることがあればやる
			break;
		case Mode::launch:
			if(time >= 6*1000){
				digitalWrite(pin::led_arduino, HIGH);
				global::mode = Mode::parachute;
			}
			break;
		case Mode::parachute:
			// 開傘判定と開傘
			break;
		case Mode::leafing:
			// リーフィング判定
			break;
	}
}
