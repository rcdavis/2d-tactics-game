#include "Rng.h"

#include <random>

namespace Rng {
	std::mt19937 s_Engine;

	void Init() {
		std::random_device rd;
		s_Engine.seed(rd());
	}

	void InitSeeded(uint32_t seed) {
		s_Engine.seed(seed);
	}

	bool RollPercentage(uint8_t percentage) {
		std::uniform_int_distribution<uint8_t> dist(0, 99);
		return dist(s_Engine) <= percentage;
	}
}
