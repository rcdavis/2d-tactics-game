#include "Tiles/TileMap.h"

bool TileSet::Init(const char* const filepath) {
	// Implementation goes here
	return true;
}

void TileSet::Destroy() {
	delete[] terrains;
	terrains = nullptr;

	terrainCount = 0;
	tileWidth = 0;
	tileHeight = 0;
	textureHandle = InvalidTextureHandle;
	columnCount = 0;
}

bool TileMap::Init(const char* const filepath) {
	// Implementation goes here
	return true;
}

void TileMap::Destroy() {
	for (uint8_t i = 0; i < layerCount; ++i)
		delete[] layers[i].tiles;

	delete[] layers;
	layers = nullptr;
	layerCount = 0;

	tileSetHandle = -1;
	tileRowCount = 0;
	tileColumnCount = 0;
	tileWidth = 0;
	tileHeight = 0;
}
