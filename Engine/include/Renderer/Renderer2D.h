#pragma once

#include "Tiles/TileHandle.h"

struct Platform;
struct Camera2D;

namespace Renderer2D {
	bool Init(Platform& platform);

	void Shutdown();

	void BeginScene(Camera2D& camera);
	void EndScene();

	void Flush();

	void DrawTileMap(Camera2D& camera, TileMapHandle tileMapHandle);
}
