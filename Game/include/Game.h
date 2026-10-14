#pragma once

#include "Platform.h"

#include "Renderer/Camera2D.h"
#include "Tiles/TileHandle.h"
#include "Tiles/TilePathing.h"
#include "Input/ActionMap.h"

#include "entt/entity/registry.hpp"
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

	entt::entity CreatePlayerTeamCharacter(glm::ivec2 cell, TileSetHandle tileSetHandle, uint16_t frameIndex = 0);

private:
	Platform mPlatform;
	ActionMap mActionMap;

	TilePathing mTilePathing;

	entt::registry mRegistry;
	entt::entity mCharacter = entt::null;

	Camera2D mCamera;

	// Cursor position in column and row
	glm::ivec2 mCursorPos {0, 0};

	float mTimeSinceKeyPressed = 0.0f;

	bool mIsRunning = false;
};
