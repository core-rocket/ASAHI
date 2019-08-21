#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include "../..//telemetry.hpp"

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

	uint32_t last_acc_time = 0.0;
	uint32_t acc_dt = 0.0;
	Vec16_t *acc, *gyro;

	while(true){
		flag = (fread(&type, 1, 1, fp) == 1);
		if(!flag) break;
		//printf("data type = %x\n", type);
		switch(type){
			case 0x01:
			case 0x02:
				size = sizeof(Vec16_t);
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
				printf("acc: ");
				printf("dt=%u", acc_dt);
				printf(", time=%u, x=%f, y=%f, z=%f\n", acc->time, static_cast<float>(acc->x)/16384.0, static_cast<float>(acc->y)/16384.0, static_cast<float>(acc->z)/16384.0);
				break;
		}
	}

	return 0;
}
