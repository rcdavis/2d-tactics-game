#pragma once

#include <unordered_map>

#include "Renderer/TextureHandle.h"
#include "glm/ext/vector_float2.hpp"

struct Glyph {
	glm::vec2 planeMin { 0.0f };
	glm::vec2 planeMax { 0.0f };
	glm::vec2 uvMin { 0.0f };
	glm::vec2 uvMax { 0.0f };
	float advance = 0.0f;
	bool hasGeometry = false;
};

struct Font {
	std::unordered_map<uint32_t, Glyph> glyphs;
	float lineHeight = 0.0f;
	float ascender = 0.0f;
	float descender = 0.0f;
	float distanceRange = 0.0f;
	uint32_t atlasSize = 0;
	TextureHandle texture = InvalidTextureHandle;
};
