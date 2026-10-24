#pragma once

#include <cstdint>

#include "Tiles/TileHandle.h"
#include "glm/ext/vector_int2.hpp"

struct GridPosition {
	glm::ivec2 cell {0, 0};
};

struct Sprite {
	TileSetHandle tileSetHandle = InvalidTileSetHandle;
	uint16_t frameIndex = 0;
};

struct Stats {
	uint8_t movementCost = 0;
};

struct PlayerTeam {};
struct EnemyTeam {};
