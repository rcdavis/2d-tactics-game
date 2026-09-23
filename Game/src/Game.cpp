#include "Game.h"

#include "Utils/Log.h"

#include "PlatformEvent.h"
#include "IWindow.h"
#include "TextureSystem.h"
#include "TextureIds.h"

// TODO: Remove in favor of the TileMap system
#include "TileIds.h"

Game::~Game() {
	Shutdown();
}

bool Game::Init() {
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

	for (const char* path : Res::Textures::Paths) {
		if (TextureSystem::Load(path) == InvalidTextureHandle) {
			LOG_ERROR("Failed to load texture: {}", path);
			return false;
		}
	}

	mTileSetHandle = static_cast<TextureHandle>(Res::Textures::Id::ToenTileSet);

	// TODO: Replace this with the TileMap system once it's integrated
	if (!mTileSet.Init(Res::Tiles::Sets::GetPath(Res::Tiles::Sets::Id::Toen))) {
		LOG_ERROR("Failed to initialize tile set");
		return false;
	}

	if (!mTileMap.Init(Res::Tiles::Maps::GetPath(Res::Tiles::Maps::Id::Toen))) {
		LOG_ERROR("Failed to initialize tile map");
		return false;
	}

	mIsRunning = true;

	LOG_INFO("Game initialized successfully");

	return true;
}

void Game::Shutdown() {
	LOG_INFO("Shutting down game");

	mTileMap.Destroy();
	mTileSet.Destroy();

	TextureSystem::Shutdown();

	mPlatform.Destroy();

	mTileSetHandle = InvalidTextureHandle;
	mIsRunning = false;
}

void Game::Run() {
	if (!Init()) {
		LOG_ERROR("Failed to initialize game");
		return;
	}

	PlatformEvent event {};
	while (mIsRunning) {
		while (mPlatform.PollEvent(event)) {
			if (event.type == PlatformEvent::Type::Quit) {
				mIsRunning = false;
				break;
			}
		}
	}
}
