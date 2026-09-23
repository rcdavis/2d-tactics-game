#pragma once

#include "Tiles/TileHandle.h"

namespace TileSystem {
	bool Init();

	void Shutdown();

	TileSetHandle LoadTileSet(const char* const path);
	TileMapHandle LoadTileMap(const char* const path);
} // namespace TileSystem
