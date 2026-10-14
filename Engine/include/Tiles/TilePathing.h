#pragma once

#include "Tiles/TileHandle.h"

class TilePathing {
public:
	struct Cell {
		uint16_t row = 0;
		uint16_t col = 0;
		uint8_t movementCost = 0;
	};

public:
	TilePathing() = default;
	~TilePathing();

	void Init(TileMapHandle tileMapHandle);
	void Destroy();

private:
	Cell* mCells = nullptr;
	uint16_t mRowCount = 0;
	uint16_t mColumnCount = 0;
};
