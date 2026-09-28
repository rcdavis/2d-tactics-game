#pragma once

#include <array>

#include "Renderer/TextureHandle.h"

#include "glm/ext/vector_float2.hpp"

struct TileTerrain {
	uint8_t movementCost = 0;
};

struct TileSet {
	TileTerrain* terrains = nullptr;
	uint16_t terrainCount = 0;

	uint16_t tileWidth = 0;
	uint16_t tileHeight = 0;

	TextureHandle textureHandle = InvalidTextureHandle;
	uint8_t columnCount = 0;

	bool Init(const char* const filepath);
	void Destroy();

	std::array<glm::vec2, 4> GetTexCoords(uint16_t tileId) const;
};

struct TileLayer {
	uint16_t* tiles = nullptr;
};

struct TileMap {
	TileLayer* layers = nullptr;
	uint8_t layerCount = 0;
	uint8_t tileSetHandle = -1;

	uint16_t tileRowCount = 0;
	uint16_t tileColumnCount = 0;

	uint16_t tileWidth = 0;
	uint16_t tileHeight = 0;

	bool Init(const char* const filepath);
	void Destroy();
};
