#include "TextureSystem.h"

#include "IRenderDevice.h"
#include "ITexture.h"

#include "Utils/Log.h"

#include <cassert>

namespace TextureSystem {
	static ITexture* s_textures[(uint8_t)Res::Textures::Id::Count] {};

	bool Init(IRenderDevice* renderDevice) {
		for (uint8_t i = 0; i < (uint8_t)Res::Textures::Id::Count; ++i) {
			const char* path = Res::Textures::GetPath((Res::Textures::Id)i);
			s_textures[i] = renderDevice->CreateTexture();
			if (!s_textures[i]->Init(path)) {
				LOG_ERROR("Failed to create texture for file \"{}\"", path);
				return false;
			}
		}

		return true;
	}

	void Shutdown() {
		for (uint8_t i = 0; i < (uint8_t)Res::Textures::Id::Count; ++i) {
			delete s_textures[i];
			s_textures[i] = nullptr;
		}
	}

	void Bind(Res::Textures::Id id, uint32_t slot) {
		assert(id < Res::Textures::Id::Count && "Invalid texture ID");
		s_textures[(uint8_t)id]->Bind(slot);
	}
}
