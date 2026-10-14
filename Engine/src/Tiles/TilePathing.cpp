#include "Tiles/TilePathing.h"

#include "Tiles/TileSystem.h"
#include "Tiles/TileMap.h"

TilePathing::~TilePathing() {
	Destroy();
}

void TilePathing::Init(TileMapHandle tileMapHandle) {
	const TileMap* const tileMap = TileSystem::GetTileMap(tileMapHandle);
	const TileSet* const tileSet = TileSystem::GetTileSet(tileMap->tileSetHandle);

	mRowCount = tileMap->tileRowCount;
	mColumnCount = tileMap->tileColumnCount;

	mCells = new Cell[mRowCount * mColumnCount];
	for (uint16_t row = 0; row < mRowCount; ++row) {
		for (uint16_t col = 0; col < mColumnCount; ++col) {
			for (int layer = tileMap->layerCount - 1; layer >= 0; --layer) {
				const uint16_t tileId = tileMap->layers[layer].tiles[row * mColumnCount + col];
				if (tileId == 0)
					continue;

				mCells[row * mColumnCount + col] = {
					.row = row,
					.col = col,
					.movementCost = tileSet->terrains[tileId].movementCost,
				};
				break;
			}
		}
	}
}

void TilePathing::Destroy() {
	delete[] mCells;
	mCells = nullptr;

	mRowCount = 0;
	mColumnCount = 0;
}
