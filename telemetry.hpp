#ifndef ASAHI_TELEMETRY_H_
#define ASAHI_TELEMETRY_H_

#define PACKED	__attribute__((__packed__))

// 論理ID
constexpr uint8_t id_bus		= 0x00;
constexpr uint8_t id_mission	= 0x01;
constexpr uint8_t id_station	= 0x64;

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

struct Flag {
	uint32_t time;
	uint8_t flag;
} PACKED;

struct GPS_altitude {
	uint32_t time;
	uint32_t altitude;
	uint32_t altitude_geo;
} PACKED;

struct GPS_time {
	uint32_t time;
	uint32_t time_int;
	uint16_t time_dec;
} PACKED;

struct GPS_vec2 {
	uint32_t time;
	uint32_t x_int;	uint16_t x_dec;
	uint32_t y_int;	uint16_t y_dec;
} PACKED;

#endif
