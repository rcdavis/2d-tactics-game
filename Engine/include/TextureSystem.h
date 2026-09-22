#pragma once

#include "TextureIds.h"

class IRenderDevice;

namespace TextureSystem {
	bool Init(IRenderDevice* renderDevice);

	void Shutdown();

	void Bind(Res::Textures::Id id, uint32_t slot = 0);
}
