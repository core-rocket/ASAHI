#include <queue>

#include "../TWE-Lite/TWE-Lite.hpp"
#include "../telemetry.hpp"

namespace twelite {
	extern TWE_Lite *twe;

	struct vec_t {
		float time;
		float x, y, z;
	};

	extern vec_t latest_acc;
	extern vec_t latest_gyro;

	extern std::queue<uint8_t> cmd_queue;
	extern std::queue<vec_t> acc, gyro;

	bool init();
	void loop();
}
