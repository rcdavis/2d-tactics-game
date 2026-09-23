#include "Renderer/TextureSystem.h"

#include "Renderer/IRenderDevice.h"
#include "Renderer/ITexture.h"

#include "Utils/Log.h"

#include <vector>
#include <filesystem>
#include <cassert>

namespace TextureSystem {
	static std::vector<ITexture*> s_textures;
	static std::vector<std::filesystem::path> s_texturePaths;
	static IRenderDevice* s_renderDevice = nullptr;

	bool Init(IRenderDevice* renderDevice) {
		if (!renderDevice) {
			LOG_ERROR("Invalid render device");
			return false;
		}

		s_renderDevice = renderDevice;

		return true;
	}

	void Shutdown() {
		for (size_t i = 0; i < std::size(s_textures); ++i) {
			delete s_textures[i];
		}
		s_textures.clear();
		s_texturePaths.clear();
		s_renderDevice = nullptr;
	}

	TextureHandle Load(const char* const path) {
		for (size_t i = 0; i < std::size(s_texturePaths); ++i) {
			if (s_texturePaths[i] == path) {
				return static_cast<TextureHandle>(i);
			}
		}

		ITexture* texture = s_renderDevice->CreateTexture();
		if (!texture->Init(path)) {
			LOG_ERROR("Failed to create texture for file \"{}\"", path);
			delete texture;
			return InvalidTextureHandle;
		}
		s_textures.push_back(texture);
		s_texturePaths.push_back(path);
		return static_cast<TextureHandle>(std::size(s_textures) - 1);
	}

	void Bind(TextureHandle id, uint32_t slot) {
		assert(id < std::size(s_textures) && "Invalid texture ID");
		s_textures[id]->Bind(slot);
	}
}
