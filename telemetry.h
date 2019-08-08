#ifndef ASAHI_TELEMETRY_H_
#define ASAHI_TELEMETRY_H_

#define PACKED	__attribute__((__packed__))

// 3軸のuint16_tの値
struct Vec16_t {
	uint32_t time;
	union {
		int16_t raw[3];
		struct {
			int16_t x, y, z;
		};
	};
} PACKED;

// 32bit float
struct Float32 {
	uint32_t time;
	union {
		uint32_t raw;
		float value;
	};
} PACKED;

#endif
