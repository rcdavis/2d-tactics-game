#pragma once

#include "Platform.h"

#include "Renderer/Camera2D.h"
#include "Renderer/FontHandle.h"
#include "Tiles/TileHandle.h"
#include "Tiles/TilePathing.h"
#include "Input/ActionMap.h"

#include "entt/entity/registry.hpp"
#include "glm/ext/vector_int2.hpp"

class Game {
public:
	enum class State : uint8_t {
		SelectCharacter,
		MovementZone,
		MovingCharacter,
		StatsMenu,
	};

public:
	Game() = default;
	~Game();

	void Run();

private:
	bool Init();
	void Shutdown();

	void Update(const float elapsedTime);
	void UpdateSelectCharacterState(const float elapsedTime);
	void UpdateMovementZoneState(const float elapsedTime);
	void UpdateMovingCharacterState(const float elapsedTime);
	void UpdateStatsMenuState(const float elapsedTime);

	void Render();
	void RenderStatsMenu();

	void OnResize(uint16_t width, uint16_t height);

	void Close();

	void UpdateCursor(const float elapsedTime);
	void FocusCamera(const glm::ivec2& cell);

	entt::entity CreatePlayerTeamArcher(glm::ivec2 cell, TileSetHandle tileSetHandle);
	entt::entity CreatePlayerTeamSoldier(glm::ivec2 cell, TileSetHandle tileSetHandle);
	entt::entity CreatePlayerTeamKnight(glm::ivec2 cell, TileSetHandle tileSetHandle);
	entt::entity CreatePlayerTeamCavalier(glm::ivec2 cell, TileSetHandle tileSetHandle);

	entt::entity CreateEnemyTeamArcher(glm::ivec2 cell, TileSetHandle tileSetHandle);
	entt::entity CreateEnemyTeamSoldier(glm::ivec2 cell, TileSetHandle tileSetHandle);
	entt::entity CreateEnemyTeamKnight(glm::ivec2 cell, TileSetHandle tileSetHandle);
	entt::entity CreateEnemyTeamCavalier(glm::ivec2 cell, TileSetHandle tileSetHandle);

private:
	Platform mPlatform;
	ActionMap mActionMap;

	TilePathing mTilePathing;

	entt::registry mRegistry;
	entt::entity mSelectedCharacter = entt::null;

	Camera2D mCamera;

	// Cursor position in column and row
	glm::ivec2 mCursorPos {0, 0};

	float mTimeSinceKeyPressed = 0.0f;

	FontHandle mFontHandle = InvalidFontHandle;

	State mState = State::SelectCharacter;
	bool mIsRunning = false;
};
