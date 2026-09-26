#pragma once

#include "Tiles/TileHandle.h"

struct TileSet;
struct TileMap;

namespace TileSystem {
	bool Init();

	void Shutdown();

	TileSetHandle LoadTileSet(const char* const path);
	TileMapHandle LoadTileMap(const char* const path);

	const TileSet* GetTileSet(TileSetHandle handle);
	const TileMap* GetTileMap(TileMapHandle handle);
} // namespace TileSystem
