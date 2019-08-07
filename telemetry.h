#ifndef ASAHI_TELEMETRY_H_
#define ASAHI_TELEMETRY_H_

#define PACKED	__attribute__((__packed__))

// 3軸のuint16_tの値
struct Vec16_t {
	union {
		uint16_t raw[3];
		struct {
			uint16_t x, y, z;
		};
	};
} PACKED;

// 32bit float
struct Float32 {
	union {
		uint32_t raw;
		float value;
	};
} PACKED;

#endif
