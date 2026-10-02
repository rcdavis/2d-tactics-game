#pragma once

#include "ShaderHandle.h"
#include "Tiles/TileHandle.h"

struct Camera2D;
class IRenderDevice;

namespace Renderer2D {
	bool Init(IRenderDevice* renderDevice, ShaderHandle quadShaderHandle);

	void Shutdown();

	void BeginScene(Camera2D& camera);
	void EndScene();

	void Flush();

	void DrawTileMap(Camera2D& camera, TileMapHandle tileMapHandle);
}
