#pragma once

#include <cstdint>
#include <span>

class IRenderDevice;

namespace TextureSystem {
	bool Init(IRenderDevice* renderDevice, std::span<const char* const> texturePaths);

	void Shutdown();

	void Bind(uint8_t id, uint32_t slot = 0);
}
