#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>

struct TileTerrainData {
	uint8_t movementCost = 1;
};

struct TileSetData {
	std::vector<TileTerrainData> terrains;
	std::filesystem::path imagePath;
	uint16_t imageWidth = 0;
	uint16_t imageHeight = 0;
	uint16_t tileWidth = 0;
	uint16_t tileHeight = 0;
	uint16_t tileCount = 0;
	uint16_t columnCount = 0;
};

struct TileSetBinaryHeader {
	uint16_t imageHandle = 0;
	uint16_t imageWidth = 0;
	uint16_t imageHeight = 0;
	uint16_t tileWidth = 0;
	uint16_t tileHeight = 0;
	uint16_t tileCount = 0;
	uint16_t columnCount = 0;
};
