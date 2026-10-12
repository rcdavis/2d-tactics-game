#pragma once

#include <cstdint>

#include "Renderer/TextureHandle.h"
#include "glm/ext/vector_int2.hpp"

struct Character {
	// Column and row position on the tile map
	glm::ivec2 position {0, 0};
	uint16_t sourceX = 0;
	uint16_t sourceY = 0;
	uint8_t sourceWidth = 0;
	uint8_t sourceHeight = 0;
	TextureHandle textureHandle = InvalidTextureHandle;
};
