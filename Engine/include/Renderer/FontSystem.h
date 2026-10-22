#pragma once

#include "Renderer/FontHandle.h"
#include "Renderer/TextureHandle.h"

struct Font;

namespace FontSystem {
	bool Load(const char* const fontFile, TextureHandle fontImage);

	Font* Get(FontHandle handle);
} // namespace FontSystem
