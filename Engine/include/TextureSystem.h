#pragma once

#include "TextureHandle.h"

class IRenderDevice;

namespace TextureSystem {
	bool Init(IRenderDevice* renderDevice);

	void Shutdown();

	TextureHandle Load(const char* const path);

	void Bind(TextureHandle id, uint32_t slot = 0);
}
