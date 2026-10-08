#pragma once

#include "Platform.h"

#include "Renderer/Camera2D.h"
#include "Input/ActionMap.h"

class Game {
public:
	Game() = default;
	~Game();

	void Run();

private:
	bool Init();
	void Shutdown();

	void Render();

	void OnResize(uint16_t width, uint16_t height);

	void Close();

private:
	Platform mPlatform;
	ActionMap mActionMap;
	Camera2D mCamera;

	bool mIsRunning = false;
};
