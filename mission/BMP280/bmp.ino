#include <Wire.h>				// i2c
#include <Adafruit_BMP280.h>	// BMPライブラリ
#include <math.h>

// ピン
namespace pin {
	constexpr uint8_t led	= 13;	// 内蔵LED
}

//
namespace sensor {
	Adafruit_BMP280	bmp;	// BMP
}

// 関数
const float get_altitude();	// BMP280の値から高度を推定する
void error();

void setup(){
	Serial.begin(9600);

	// i2c初期化
	Wire.begin();
	if(!sensor::bmp.begin()){	// BMP280初期化
		error();
	}

	// LED初期化
	pinMode(pin::led, OUTPUT);
	digitalWrite(pin::led, LOW);
}

void loop(){
	const auto altitude = get_altitude();
	Serial.println(altitude);
}

const float get_altitude(){
	constexpr float p_0 = 1013.0;							// 大気圧(hPa)
	const float t = sensor::bmp.readTemperature();			// 気温(C)
	const float p = sensor::bmp.readPressure() / 100.0f;	// 気圧(hPa)
	const float a = (pow(p_0/p, 1/5.257) - 1)*(t+273.15) / 0.0065;
	return a;
}

void error(){
	Serial.println("error");
	while(true){
		digitalWrite(pin::led, HIGH);
		delay(100);
		digitalWrite(pin::led, LOW);
		delay(100);
	}
}
