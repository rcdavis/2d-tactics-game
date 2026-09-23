#include "Tiles/TileMap.h"

#include <fstream>

#include "Utils/Log.h"

namespace {
	struct TileSetBinaryHeader {
		char magic[4] = {'T', 'S', 'X', 'B'};
		uint16_t version = 1;
		uint16_t imageHandle = 0;
		uint16_t imageWidth = 0;
		uint16_t imageHeight = 0;
		uint16_t tileWidth = 0;
		uint16_t tileHeight = 0;
		uint16_t tileCount = 0;
		uint16_t columnCount = 0;
	};
} // namespace

bool TileSet::Init(const char* const filepath) {
	// Implementation goes here
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		LOG_ERROR("Failed to open tile set file \"{}\"", filepath);
		return false;
	}

	TileSetBinaryHeader header;
	file.read(reinterpret_cast<char*>(&header), sizeof(TileSetBinaryHeader));

	if (strncmp(header.magic, "TSXB", 4) != 0) {
		LOG_ERROR("Invalid magic in tile set file \"{}\"", filepath);
		return false;
	}

	if (header.version != 1) {
		LOG_ERROR("Unsupported tile set binary version in \"{}\"", filepath);
		return false;
	}

	terrains = new TileTerrain[header.tileCount];
	terrainCount = header.tileCount;

	tileWidth = header.tileWidth;
	tileHeight = header.tileHeight;
	textureHandle = static_cast<TextureHandle>(header.imageHandle);
	columnCount = header.columnCount;

	file.read(reinterpret_cast<char*>(terrains), sizeof(TileTerrain) * header.tileCount);

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
