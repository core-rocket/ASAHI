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
				printf("unknwon type\n");
				break;
		}
		flag = (fread(buf, 1, size, fp) == size);
		if(!flag) break;

		switch(type){
			case 0x01:
				acc = reinterpret_cast<Vec16_t*>(buf);
				printf("acc: time=%u, x=%u, y=%u, z=%u\n", acc->time, acc->x, acc->y, acc->z);
				break;
		}
	}

	return 0;
}
