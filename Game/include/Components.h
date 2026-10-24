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
	uint8_t level = 1;

	int8_t curHp = 0;
	int8_t maxHp = 0;

	uint8_t strength = 0;
	uint8_t magic = 0;
	uint8_t skill = 0;
	uint8_t speed = 0;
	uint8_t luck = 0;
	uint8_t defense = 0;
	uint8_t resistance = 0;

	uint8_t movement = 0;
};

struct PlayerTeam {};
struct EnemyTeam {};
