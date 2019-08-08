// ピン番号
namespace pin {
	constexpr uint8_t flight	= 2;
	constexpr uint8_t led		= 13;	// 内蔵LED
}

// グローバル変数
namespace global {
	// 割り込みハンドラで弄る変数
	volatile unsigned launch_time = 0;	// 離床時刻
}

// 割り込みハンドラ
void flightpin_handler();

void setup(){
	Serial.begin(9600);

	// フライトピン --- GND
	pinMode(pin::flight, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(pin::flight), flightpin_handler, CHANGE);

	// LED
	pinMode(pin::led, OUTPUT);
	digitalWrite(pin::led, LOW);
}

void loop(){
	const auto &launch_time = global::launch_time;

	if(launch_time != 0){
		digitalWrite(pin::led, HIGH);
	}
}

void flightpin_handler(){
	global::launch_time = millis();
}
