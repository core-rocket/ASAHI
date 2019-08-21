#include <queue>

#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.hpp"

namespace twelite {
	extern TWE_Lite *twe;

	struct vec_t {
		float time;
		float x, y, z;
	};

	struct double_t {
		float time;
		double val;
	};

	extern vec_t latest_acc;
	extern vec_t latest_gyro;

	extern std::queue<uint8_t> cmd_queue;
	extern std::queue<std::string> log;
	extern std::queue<vec_t> acc, gyro;
	extern std::queue<double_t> bus_temp;
	extern std::queue<double_t> mission_temp;
	extern std::queue<double_t> pressure;
	extern std::queue<double_t> altitude;
	extern std::queue<GPS_time> gps_time;
	extern std::queue<GPS_vec2> gps_pos;
	extern std::queue<GPS_vec2> gps_alt;

	bool init();
	void loop();
}
