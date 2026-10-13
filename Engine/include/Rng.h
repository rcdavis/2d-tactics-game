#pragma once

#include <cstdint>

namespace Rng {
	void Init();
	void InitSeeded(uint32_t seed);

	bool RollPercentage(uint8_t percentage);
}
