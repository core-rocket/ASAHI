#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include "../..//telemetry.hpp"

#define ACC_LSB		4096.0
#define GYRO_LSB	65.5

int main(int argc, char **argv){
	FILE *fp = std::fopen(argv[1], "rb");
	if(fp == nullptr){
		puts("file open failed");
		return -1;
	}

	uint8_t type;
	uint8_t buf[30];
	size_t num = 0;
	size_t size= 0;
	bool flag = true;

	uint32_t last_acc_time = 0.0, last_gyro_time = 0.0, last_temp_time = 0.0;
	uint32_t acc_dt = 0.0, gyro_dt= 0.0, temp_dt = 0.0;
	Vec16_t *acc, *gyro;
	Value16 *temp;

	while(true){
		flag = (fread(&type, 1, 1, fp) == 1);
		if(!flag) break;
		//printf("data type = %x\n", type);
		switch(type){
			case 0x01:
			case 0x02:
				size = sizeof(Vec16_t);
				break;
			case 0x03:
				size = sizeof(Value16);
				break;
			case 0x08:
				size = sizeof(GPS_time);
				break;
			case 0x09:
			case 0x0a:
				size = sizeof(GPS_vec2);
				break;
			default:
				printf("unknwon type: %d\n", (int)type);
				break;
		}
		flag = (fread(buf, 1, size, fp) == size);
		if(!flag) break;

		switch(type){
			case 0x01:
				acc = reinterpret_cast<Vec16_t*>(buf);
				acc_dt = acc->time - last_acc_time;
				last_acc_time = acc->time;
				printf("acc, %f, %u, ", static_cast<float>(acc->time)/1000.0f, acc_dt);
				printf("%f, %f, %f\n", static_cast<float>(acc->x)/ACC_LSB, static_cast<float>(acc->y)/ACC_LSB, static_cast<float>(acc->z)/ACC_LSB);
				break;
			case 0x02:
				gyro = reinterpret_cast<Vec16_t*>(buf);
				gyro_dt= gyro->time - last_gyro_time;
				last_gyro_time = gyro->time;
				printf("gyro, %f, %u, ", static_cast<float>(gyro->time)/1000.0f, gyro_dt);
				printf("%f, %f, %f\n", static_cast<float>(gyro->x)/GYRO_LSB, static_cast<float>(gyro->y)/GYRO_LSB, static_cast<float>(gyro->z)/GYRO_LSB);
				break;
			case 0x03:
				temp = reinterpret_cast<Value16*>(buf);
				temp_dt = temp->time;
				last_temp_time = temp->time;
				printf("temperature, %f, %u, ", static_cast<float>(temp->time)/1000.0f, temp_dt);
				printf("%f\n", (static_cast<float>(temp->val)+12412.0)/340.0);
				break;
		}
	}

	return 0;
}
