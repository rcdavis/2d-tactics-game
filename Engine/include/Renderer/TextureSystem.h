#pragma once

#include "TextureHandle.h"

class IRenderDevice;
class ITexture;

namespace TextureSystem {
	bool Init(IRenderDevice* renderDevice);

	void Shutdown();

	TextureHandle Load(const char* const path);

	ITexture* GetTexture(TextureHandle id);

	void Bind(TextureHandle id, uint32_t slot = 0);
}
