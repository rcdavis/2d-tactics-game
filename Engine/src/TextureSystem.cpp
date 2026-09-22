#include "TextureSystem.h"

#include "IRenderDevice.h"
#include "ITexture.h"

#include "Utils/Log.h"

#include <vector>
#include <cassert>

namespace TextureSystem {
	static std::vector<ITexture*> s_textures;

	bool Init(IRenderDevice* renderDevice, std::span<const char* const> texturePaths) {
		s_textures.reserve(std::size(texturePaths));
		for (size_t i = 0; i < std::size(texturePaths); ++i) {
			ITexture* texture = renderDevice->CreateTexture();
			if (!texture->Init(texturePaths[i])) {
				LOG_ERROR("Failed to create texture for file \"{}\"", texturePaths[i]);
				delete texture;
				return false;
			}
			s_textures.push_back(texture);
		}

		return true;
	}

	void Shutdown() {
		for (size_t i = 0; i < std::size(s_textures); ++i) {
			delete s_textures[i];
		}
		s_textures.clear();
	}

	void Bind(uint8_t id, uint32_t slot) {
		assert(id < std::size(s_textures) && "Invalid texture ID");
		s_textures[id]->Bind(slot);
	}
}
