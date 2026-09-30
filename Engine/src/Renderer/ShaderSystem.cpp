#include "Renderer/ShaderSystem.h"

#include "Renderer/IShader.h"
#include "Renderer/IRenderDevice.h"

#include "Utils/Log.h"

#include <vector>
#include <filesystem>
#include <cassert>

namespace ShaderSystem {
	static std::vector<IShader*> s_Shaders;
	static std::vector<std::pair<std::filesystem::path, std::filesystem::path>> s_ShaderPaths;
	static IRenderDevice* s_RenderDevice = nullptr;

	bool Init(IRenderDevice* renderDevice) {
		if (!renderDevice) {
			LOG_ERROR("Invalid render device");
			return false;
		}

		s_RenderDevice = renderDevice;
		return true;
	}

	void Shutdown() {
		for (size_t i = 0; i < std::size(s_Shaders); ++i)
			delete s_Shaders[i];

		s_Shaders.clear();
		s_ShaderPaths.clear();
		s_RenderDevice = nullptr;
	}

	ShaderHandle Load(const char* const vertexFilepath, const char* const fragmentFilepath) {
		for (size_t i = 0; i < std::size(s_ShaderPaths); ++i) {
			if (s_ShaderPaths[i].first == vertexFilepath && s_ShaderPaths[i].second == fragmentFilepath) {
				return static_cast<ShaderHandle>(i);
			}
		}

		IShader* shader = s_RenderDevice->CreateShader();
		if (!shader->Init(vertexFilepath, fragmentFilepath)) {
			LOG_ERROR("Failed to create shader for files \"{}\" and \"{}\"", vertexFilepath, fragmentFilepath);
			delete shader;
			return InvalidShaderHandle;
		}

		s_Shaders.push_back(shader);
		s_ShaderPaths.push_back({vertexFilepath, fragmentFilepath});
		return static_cast<ShaderHandle>(std::size(s_Shaders) - 1);
	}

	IShader* GetShader(ShaderHandle id) {
		assert(id >= 0 && static_cast<size_t>(id) < std::size(s_Shaders));
		return s_Shaders[id];
	}
}
