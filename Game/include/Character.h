#pragma once

#include <cstdint>

#include "Renderer/TextureHandle.h"
#include "glm/ext/vector_int2.hpp"

struct Character {
	// Column and row position on the tile map
	glm::ivec2 position {0, 0};
	uint16_t tileId = 0;
	TextureHandle textureHandle = InvalidTextureHandle;
};
