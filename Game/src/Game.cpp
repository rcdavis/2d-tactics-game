#include "Game.h"

#include "Utils/Log.h"

#include "PlatformEvent.h"

Game::~Game() {
	Shutdown();
}

bool Game::Init() {
	constexpr WindowCreateInfo windowCreateInfo {
		.title = "2D Tactics Game",
		.width = 800,
		.height = 600,
		.isResizable = false,
		.isFullscreen = false,
	};

	if (!mPlatform.Init(windowCreateInfo)) {
		LOG_ERROR("Failed to initialize platform");
		return false;
	}

	mIsRunning = true;

	LOG_INFO("Game initialized successfully");

	return true;
}

void Game::Shutdown() {
	LOG_INFO("Shutting down game");

	mPlatform.Destroy();
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
