#pragma once

#include "Platform.h"

#include "Renderer/Camera2D.h"
#include "Input/ActionMap.h"

#include "glm/ext/vector_int2.hpp"

class Game {
public:
	Game() = default;
	~Game();

	void Run();

private:
	bool Init();
	void Shutdown();

	void Update(float elapsedTime);
	void Render();

	void OnResize(uint16_t width, uint16_t height);

	void Close();

private:
	Platform mPlatform;
	ActionMap mActionMap;
	Camera2D mCamera;

	// Cursor position in column and row
	glm::ivec2 mCursorPos {0, 0};

	bool mIsRunning = false;
};
