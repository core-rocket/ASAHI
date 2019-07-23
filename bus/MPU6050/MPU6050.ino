// MPU6050のテスト用コード
#include <Wire.h>
#include "MPU6050.hpp"

MPU6050 mpu;

void setup(){
	Serial.begin(9600);
	Wire.begin();

	// 's'が入力されるまで待つ
	while(true){
		if(Serial.available() < 0) continue;
		if(Serial.read() == 's') break;
	}

	Serial.println("MPU6050");
	mpu.init();
}

void loop(){
	// MPU6050から加速度・温度・ジャイロデータを取得
	auto data = mpu.get_data();

	auto start = millis();
	for(int i=0;i<10;i++){
		data = mpu.get_data();
	}
	auto end = millis();
	// 1度のデータ取得にかかる時間
//	Serial.println((end-start)/10.0);

	// エラーだったらリセットをかける
	if(mpu.get_error() != MPU6050::Error::No){
		Serial.println("error");
		TWCR=0;
		Wire.begin();
		mpu.init();
	}

	// 生のデータから実際の数値を計算する
	// TODO	この式もMPU6050クラスで扱うようにするべきか考える
	//		この数値はMPU6050の設定で変更できるので，なんらかの方法での抽象化は行うべき．
	//		しかし，あまり浮動小数の計算はしたくない(というか，機体上で計算する必要はほぼ無いはず)
	//		できればconstexprとかTMPとか使ってコンパイル時にどうにかしたい．
	//		特に思いつかなかったら，ifdefで選択できるようにした上で関数を生やす．
	for(int i=0;i<3;i++){
		Serial.print(static_cast<float>(data.acc[i]) / 16384.0);
		Serial.print(" ");
	}
	for(int i=0;i<3;i++){
		Serial.print(static_cast<float>(data.gyro[i]) / 131.0);
		Serial.print(" ");
	}
	Serial.println((static_cast<float>(data.temperature)+12412.0)/340.0);

	delay(500);
}
