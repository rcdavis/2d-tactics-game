#pragma once

#include "Platform.h"

#include "TextureHandle.h"

// TODO: Remove in favor of the TileMap system
#include "Tiles/TileMap.h"

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

	TileSet mTileSet;

	TextureHandle mTileSetHandle = InvalidTextureHandle;

	bool mIsRunning = false;
};
