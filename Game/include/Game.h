#pragma once

#include "Platform.h"

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

	bool mIsRunning = false;
};
