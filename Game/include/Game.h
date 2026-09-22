#pragma once

#include "Platform.h"

#include "TextureHandle.h"

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

	TextureHandle mTileSetHandle = InvalidTextureHandle;

	bool mIsRunning = false;
};
