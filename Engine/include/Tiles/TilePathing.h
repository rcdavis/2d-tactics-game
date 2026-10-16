#pragma once

#include "Tiles/TileHandle.h"

#include <vector>

class TilePathing {
public:
	struct Cell {
		uint16_t row = 0;
		uint16_t col = 0;
		uint8_t movementCost = 1;
	};

public:
	TilePathing() = default;
	~TilePathing();

	void Init(TileMapHandle tileMapHandle);
	void Destroy();

	std::vector<Cell> FindPath(uint16_t startRow, uint16_t startCol, uint16_t endRow, uint16_t endCol);

	Cell GetCell(uint16_t row, uint16_t col) const { return mCells[static_cast<uint32_t>(row) * mColumnCount + col]; }

private:
	Cell* mCells = nullptr;
	uint16_t mRowCount = 0;
	uint16_t mColumnCount = 0;
};
