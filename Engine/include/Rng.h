#pragma once

#include <cstdint>

namespace Rng {
	bool Init();
	bool InitSeeded(uint32_t seed);

	bool RollPercentage(uint8_t percentage);
}
