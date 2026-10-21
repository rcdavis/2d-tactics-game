#include "Game.h"

#include "Utils/Log.h"

#include "PlatformEvent.h"
#include "IWindow.h"
#include "Renderer/IRenderDevice.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/TextureSystem.h"
#include "Renderer/ShaderSystem.h"
#include "Tiles/TileSystem.h"
#include "Tiles/TileMap.h"
#include "Input/Input.h"

#include "Timer.h"
#include "TextureIds.h"
#include "ShaderIds.h"
#include "TileIds.h"
#include "Components.h"
#include "Rng.h"
#include "Localization.h"
#include "StringIds.h"

#include <filesystem>

Game::~Game() {
	Shutdown();
}

bool Game::Init() {
	constexpr ShaderHandle quadShaderHandle = static_cast<ShaderHandle>(Res::Shaders::Vertex::Id::TextureQuad);
	constexpr TileMapHandle tileMapHandle = static_cast<TileMapHandle>(Res::Tiles::Maps::Id::Toen);
	constexpr TileSetHandle tileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);

	constexpr WindowCreateInfo windowCreateInfo {
		.title = "2D Tactics Game",
		.width = 800,
		.height = 600,
		.graphicsAPI = GraphicsAPI::OpenGL,
		.isResizable = false,
		.isFullscreen = false,
		.useVSync = true,
	};

	if (!mPlatform.Init(windowCreateInfo)) {
		LOG_ERROR("Failed to initialize platform");
		return false;
	}

	if (!TextureSystem::Init(mPlatform.renderDevice)) {
		LOG_ERROR("Failed to initialize texture system");
		return false;
	}

	if (!ShaderSystem::Init(mPlatform.renderDevice)) {
		LOG_ERROR("Failed to initialize shader system");
		return false;
	}

	for (const char* path : Res::Textures::Paths) {
		if (TextureSystem::Load(path) == InvalidTextureHandle) {
			LOG_ERROR("Failed to load texture: {}", path);
			return false;
		}
	}

	if (!TileSystem::Init()) {
		LOG_ERROR("Failed to initialize tile system");
		return false;
	}

	for (const char* path : Res::Tiles::Sets::Paths) {
		if (TileSystem::LoadTileSet(path) == InvalidTileSetHandle) {
			LOG_ERROR("Failed to load tile set: {}", path);
			return false;
		}
	}

	for (const char* path : Res::Tiles::Maps::Paths) {
		if (TileSystem::LoadTileMap(path) == InvalidTileMapHandle) {
			LOG_ERROR("Failed to load tile map: {}", path);
			return false;
		}
	}

	constexpr uint32_t numShaderPaths = static_cast<uint32_t>(Res::Shaders::Vertex::Paths.size());
	for (uint32_t i = 0; i < numShaderPaths; ++i) {
		const char* vs = Res::Shaders::Vertex::GetPath(static_cast<Res::Shaders::Vertex::Id>(i));
		const char* fs = Res::Shaders::Fragment::GetPath(static_cast<Res::Shaders::Fragment::Id>(i));
		if (ShaderSystem::Load(vs, fs) == InvalidShaderHandle) {
			LOG_ERROR("Failed to load shader: {} {}", vs, fs);
			return false;
		}
	}

	Rng::Init();

	mCamera.Init(windowCreateInfo.width, windowCreateInfo.height);

	if (!Renderer2D::Init(mPlatform.renderDevice, quadShaderHandle)) {
		LOG_ERROR("Failed to initialize Renderer2D");
		return false;
	}

	mPlatform.renderDevice->SetClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	mPlatform.renderDevice->EnableDepthTest(true);

	const std::filesystem::path preferencesPath = mPlatform.GetPreferencesPath("rcdavis", "2d-tactics-game");
	mActionMap = ActionMap::LoadBindings(preferencesPath / "bindings.json");

	mTimeSinceKeyPressed = 0.0f;

	mTilePathing.Init(tileMapHandle);

	CreatePlayerTeamArcher({7, 8}, tileSetHandle);
	CreatePlayerTeamSpearman({0, 0}, tileSetHandle);
	CreatePlayerTeamKnight({12, 12}, tileSetHandle);
	CreatePlayerTeamPaladin({5, 5}, tileSetHandle);

	mState = State::SelectCharacter;
	mSelectedCharacter = entt::null;
	mCursorPos = {0, 0};

	if (!Localization::Load("res/strings/en.strbin")) {
		LOG_ERROR("Failed to initialize localization");
		return false;
	}

	constexpr StringHandle menuPauseStringHandle = static_cast<StringHandle>(Res::Strings::Id::MENU_PAUSE);
	constexpr StringHandle menuQuitStringHandle = static_cast<StringHandle>(Res::Strings::Id::MENU_QUIT);
	constexpr StringHandle itemCountStringHandle = static_cast<StringHandle>(Res::Strings::Id::ITEM_COUNT);

	LOG_INFO("Test localization MENU_PAUSE: {}", Localization::GetString(menuPauseStringHandle));
	LOG_INFO("Test localization MENU_QUIT: {}", Localization::GetString(menuQuitStringHandle));
	LOG_INFO("Test localization ITEM_COUNT 1: {}", Localization::GetPlural(itemCountStringHandle, 1));
	LOG_INFO("Test localization ITEM_COUNT 2: {}", Localization::GetPlural(itemCountStringHandle, 2));

	mIsRunning = true;

	LOG_INFO("Game initialized successfully");

	return true;
}

void Game::Shutdown() {
	LOG_INFO("Shutting down game");

	mTilePathing.Destroy();

	mRegistry.clear();
	mSelectedCharacter = entt::null;

	Renderer2D::Shutdown();
	TileSystem::Shutdown();
	TextureSystem::Shutdown();
	ShaderSystem::Shutdown();

	mPlatform.Destroy();

	mIsRunning = false;
}

void Game::Run() {
	if (!Init()) {
		LOG_ERROR("Failed to initialize game");
		return;
	}

	Timer timer;
	timer.Init();

	PlatformEvent event {};
	while (mIsRunning) {
		Input::BeginFrame();

		while (mPlatform.PollEvent(event)) {
			if (event.type == PlatformEvent::Type::Quit) {
				Close();
				break;
			}

			Input::HandleEvent(event);

			switch (event.type) {
			case PlatformEvent::Type::WindowPixelSizeChanged:
				OnResize(event.windowSize.width, event.windowSize.height);
				break;
			default:
				break;
			}
		}
		mActionMap.Update();

		Update(timer.GetElapsedTime());

		Render();
	}
}

void Game::Update(const float elapsedTime) {
	UpdateCursor(elapsedTime);
	FocusCamera(mCursorPos);

	switch (mState) {
	case State::SelectCharacter:
	{
		if (mActionMap.IsPressed(Action::Select)) {
			auto view = mRegistry.view<GridPosition, PlayerTeam>();
			for (auto entity : view) {
				const auto& gridPosition = view.get<GridPosition>(entity);
				if (gridPosition.cell.x == mCursorPos.x && gridPosition.cell.y == mCursorPos.y) {
					mSelectedCharacter = entity;
					mState = State::MovementZone;
					break;
				}
			}
		}
	}
	break;

	case State::MovementZone:
	{
		if (mActionMap.IsPressed(Action::Select)) {
			auto& selectedCharPos = mRegistry.get<GridPosition>(mSelectedCharacter);
			const auto& stats = mRegistry.get<Stats>(mSelectedCharacter);
			const auto tiles = mTilePathing.FindMovementZone(
				selectedCharPos.cell.y,
				selectedCharPos.cell.x,
				stats.movementCost);
			for (const TilePathing::Cell& cell : tiles) {
				if (cell.col == mCursorPos.x && cell.row == mCursorPos.y) {
					bool isOccupied = false;
					auto positionedEntities = mRegistry.view<GridPosition>();
					for (const auto entity : positionedEntities) {
						if (entity == mSelectedCharacter) {
							continue;
						}

						const auto& gridPosition = positionedEntities.get<GridPosition>(entity);
						if (gridPosition.cell.x == mCursorPos.x && gridPosition.cell.y == mCursorPos.y) {
							isOccupied = true;
							break;
						}
					}

					if (!isOccupied) {
						selectedCharPos.cell = { mCursorPos.x, mCursorPos.y };
					}
					break;
				}
			}

			mSelectedCharacter = entt::null;
			mState = State::SelectCharacter;
		} else if (mActionMap.IsPressed(Action::Cancel)) {
			mSelectedCharacter = entt::null;
			mState = State::SelectCharacter;
		}
	}
	break;

	case State::MovingCharacter:
	{
		// TODO: Implement moving character state functionality.
	}
	break;
	}

	if (mActionMap.IsPressed(Action::Pause)) {
		// TODO: Implement pause menu functionality.
		Close();
	}
}

void Game::Render() {
	Renderer2D::Clear(ClearFlags::Color | ClearFlags::Depth);

	Renderer2D::BeginScene(mCamera);

	constexpr TextureHandle selectionRingTextureHandle = static_cast<TextureHandle>(Res::Textures::Id::SelectionRing);
	constexpr TileMapHandle tileMapHandle = static_cast<TileMapHandle>(Res::Tiles::Maps::Id::Toen);
	constexpr TileSetHandle tileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);
	const TileMap* const tileMap = TileSystem::GetTileMap(tileMapHandle);

	Renderer2D::DrawTexturedQuad(
		{ mCursorPos.x * tileMap->tileWidth, mCursorPos.y * tileMap->tileHeight, 0.6f },
		{ tileMap->tileWidth, tileMap->tileHeight },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		selectionRingTextureHandle);
	Renderer2D::DrawTileMap(mCamera, tileMapHandle);

	mRegistry.view<GridPosition, Sprite>().each([tileMap](const auto& gridPosition, const auto& sprite) {
		Renderer2D::DrawTile(
			{ gridPosition.cell.x * tileMap->tileWidth, gridPosition.cell.y * tileMap->tileHeight, 0.5f },
			{ tileMap->tileWidth, tileMap->tileHeight },
			sprite.frameIndex,
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			sprite.tileSetHandle);
	});

	if (mState == State::MovementZone && mSelectedCharacter != entt::null) {
		Renderer2D::Flush();

		mPlatform.renderDevice->EnableBlending(true);

		const auto& gridPosition = mRegistry.get<GridPosition>(mSelectedCharacter);
		const auto& stats = mRegistry.get<Stats>(mSelectedCharacter);

		const auto movementZone = mTilePathing.FindMovementZone(
			gridPosition.cell.y,
			gridPosition.cell.x,
			stats.movementCost);
		for (const TilePathing::Cell& cell : movementZone) {
			Renderer2D::DrawQuad(
				{ cell.col * tileMap->tileWidth, cell.row * tileMap->tileHeight, 0.3f },
				{ tileMap->tileWidth, tileMap->tileHeight },
				{ 0.0f, 0.0f, 0.75f, 0.5f });
		}

		const auto tilePath = mTilePathing.FindPath(
			gridPosition.cell.y,
			gridPosition.cell.x,
			mCursorPos.y,
			mCursorPos.x,
			stats.movementCost);
		for (const TilePathing::Cell& cell : tilePath) {
			Renderer2D::DrawQuad(
				{ cell.col * tileMap->tileWidth, cell.row * tileMap->tileHeight, 0.4f },
				{ tileMap->tileWidth, tileMap->tileHeight },
				{ 0.75f, 0.0f, 0.0f, 0.5f });
		}

		Renderer2D::Flush();

		mPlatform.renderDevice->EnableBlending(false);
	}

	Renderer2D::EndScene();

	Renderer2D::Present();
}

void Game::OnResize(uint16_t width, uint16_t height) {
	mPlatform.renderDevice->SetViewport(0, 0, width, height);
}

void Game::Close() {
	const std::filesystem::path prefPath = mPlatform.GetPreferencesPath("rcdavis", "2d-tactics-game");
	mActionMap.SaveBindings(prefPath / "bindings.json");
	mIsRunning = false;
}

void Game::UpdateCursor(const float elapsedTime) {
	constexpr float KeyPressInterval = 0.1f;
	constexpr TileMapHandle tileMapHandle = static_cast<TileMapHandle>(Res::Tiles::Maps::Id::Toen);
	const TileMap* const tileMap = TileSystem::GetTileMap(tileMapHandle);

	mTimeSinceKeyPressed += elapsedTime;

	if (mTimeSinceKeyPressed > KeyPressInterval) {
		if (mActionMap.IsDown(Action::MoveUp)) {
			if (mCursorPos.y > 0)
				mCursorPos.y--;
			mTimeSinceKeyPressed = 0.0f;
		} else if (mActionMap.IsDown(Action::MoveDown)) {
			if (mCursorPos.y < tileMap->tileRowCount - 1)
				mCursorPos.y++;
			mTimeSinceKeyPressed = 0.0f;
		} else if (mActionMap.IsDown(Action::MoveLeft)) {
			if (mCursorPos.x > 0)
				mCursorPos.x--;
			mTimeSinceKeyPressed = 0.0f;
		} else if (mActionMap.IsDown(Action::MoveRight)) {
			if (mCursorPos.x < tileMap->tileColumnCount - 1)
				mCursorPos.x++;
			mTimeSinceKeyPressed = 0.0f;
		}
	}
}

void Game::FocusCamera(const glm::ivec2& cell) {
	constexpr TileMapHandle tileMapHandle = static_cast<TileMapHandle>(Res::Tiles::Maps::Id::Toen);
	const TileMap* const tileMap = TileSystem::GetTileMap(tileMapHandle);

	const glm::vec2 worldEndPos = {
		tileMap->tileColumnCount * tileMap->tileWidth,
		tileMap->tileRowCount * tileMap->tileHeight
	};

	mCamera.position = {
		cell.x * tileMap->tileWidth - mCamera.width / 2.0f,
		cell.y * tileMap->tileHeight - mCamera.height / 2.0f
	};

	if (mCamera.position.x < 0.0f)
		mCamera.position.x = 0.0f;
	else if (mCamera.position.x + mCamera.width > worldEndPos.x)
		mCamera.position.x = worldEndPos.x - mCamera.width;

	if (mCamera.position.y < 0.0f)
		mCamera.position.y = 0.0f;
	else if (mCamera.position.y + mCamera.height > worldEndPos.y)
		mCamera.position.y = worldEndPos.y - mCamera.height;
}

entt::entity Game::CreatePlayerTeamArcher(glm::ivec2 cell, TileSetHandle tileSetHandle) {
	constexpr uint16_t blueArcherFrameIndex = 113;
	constexpr uint8_t blueArcherMovementCost = 6;

	entt::entity archer = mRegistry.create();
	mRegistry.emplace<GridPosition>(archer, cell);
	mRegistry.emplace<Sprite>(archer, tileSetHandle, blueArcherFrameIndex);
	mRegistry.emplace<PlayerTeam>(archer);
	mRegistry.emplace<Stats>(archer, blueArcherMovementCost);
	return archer;
}

entt::entity Game::CreatePlayerTeamSpearman(glm::ivec2 cell, TileSetHandle tileSetHandle) {
	constexpr uint16_t blueSpearFrameIndex = 114;
	constexpr uint8_t blueSpearMovementCost = 7;

	entt::entity spearman = mRegistry.create();
	mRegistry.emplace<GridPosition>(spearman, cell);
	mRegistry.emplace<Sprite>(spearman, tileSetHandle, blueSpearFrameIndex);
	mRegistry.emplace<PlayerTeam>(spearman);
	mRegistry.emplace<Stats>(spearman, blueSpearMovementCost);
	return spearman;
}

entt::entity Game::CreatePlayerTeamKnight(glm::ivec2 cell, TileSetHandle tileSetHandle) {
	constexpr uint16_t blueKnightFrameIndex = 115;
	constexpr uint8_t blueKnightMovementCost = 5;

	entt::entity knight = mRegistry.create();
	mRegistry.emplace<GridPosition>(knight, cell);
	mRegistry.emplace<Sprite>(knight, tileSetHandle, blueKnightFrameIndex);
	mRegistry.emplace<PlayerTeam>(knight);
	mRegistry.emplace<Stats>(knight, blueKnightMovementCost);
	return knight;
}

entt::entity Game::CreatePlayerTeamPaladin(glm::ivec2 cell, TileSetHandle tileSetHandle) {
	constexpr uint16_t bluePaladinFrameIndex = 116;
	constexpr uint8_t bluePaladinMovementCost = 8;

	entt::entity paladin = mRegistry.create();
	mRegistry.emplace<GridPosition>(paladin, cell);
	mRegistry.emplace<Sprite>(paladin, tileSetHandle, bluePaladinFrameIndex);
	mRegistry.emplace<PlayerTeam>(paladin);
	mRegistry.emplace<Stats>(paladin, bluePaladinMovementCost);
	return paladin;
}
