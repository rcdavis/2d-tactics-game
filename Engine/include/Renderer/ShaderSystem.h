#pragma once

#include "Renderer/ShaderHandle.h"

class IRenderDevice;
class IShader;

namespace ShaderSystem {
	bool Init(IRenderDevice* renderDevice);

	void Shutdown();

	ShaderHandle Load(const char* const vertexFilepath, const char* const fragmentFilepath);

	IShader* GetShader(ShaderHandle id);
}
