#pragma once

#include "Platform.h"

#include "Renderer/Camera2D.h"
#include "Renderer/TextureHandle.h"
#include "Tiles/TileHandle.h"

class Game {
public:
	Game() = default;
	~Game();

	void Run();

private:
	bool Init();
	void Shutdown();

private:
	Platform mPlatform;
	Camera2D mCamera;

	TileSetHandle mTileSetHandle = InvalidTileSetHandle;
	TileMapHandle mTileMapHandle = InvalidTileMapHandle;

	TextureHandle mTextureHandle = InvalidTextureHandle;

	bool mIsRunning = false;
};
