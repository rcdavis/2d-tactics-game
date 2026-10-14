#pragma once

#include "Tiles/TileHandle.h"

#include <vector>

class TilePathing {
public:
	struct Cell {
		uint16_t row = 0;
		uint16_t col = 0;
		uint8_t movementCost = 0;
	};

public:
	TilePathing() = default;

	void SetTileMap(TileMapHandle tileMapHandle);

private:
	std::vector<Cell> mCells;
	uint32_t mRowCount = 0;
	uint32_t mColumnCount = 0;
};
