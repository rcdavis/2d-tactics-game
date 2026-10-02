#include "Game.h"

#include "Utils/Log.h"

#include "PlatformEvent.h"
#include "IWindow.h"
#include "Renderer/IRenderDevice.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/TextureSystem.h"
#include "Renderer/ShaderSystem.h"
#include "Tiles/TileSystem.h"

#include "TextureIds.h"
#include "ShaderIds.h"
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

	mTileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);
	mTileMapHandle = static_cast<TileMapHandle>(Res::Tiles::Maps::Id::Toen);
	mTextureHandle = static_cast<TextureHandle>(Res::Textures::Id::ToenTileSet);

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

	constexpr const char* vs = Res::Shaders::Vertex::GetPath(Res::Shaders::Vertex::Id::ColoredQuad);
	constexpr const char* fs = Res::Shaders::Fragment::GetPath(Res::Shaders::Fragment::Id::ColoredQuad);
	if (ShaderSystem::Load(vs, fs) == InvalidShaderHandle) {
		LOG_ERROR("Failed to load shaders: {} {}", vs, fs);
		return false;
	}

	constexpr ShaderHandle quadShaderHandle = static_cast<ShaderHandle>(0);

	mCamera.SetProjection(0.0f, (float)windowCreateInfo.width, 0.0f, (float)windowCreateInfo.height);

	if (!Renderer2D::Init(mPlatform.renderDevice, quadShaderHandle)) {
		LOG_ERROR("Failed to initialize Renderer2D");
		return false;
	}

	mIsRunning = true;

	LOG_INFO("Game initialized successfully");

	return true;
}

void Game::Shutdown() {
	LOG_INFO("Shutting down game");

	Renderer2D::Shutdown();
	TileSystem::Shutdown();
	TextureSystem::Shutdown();
	ShaderSystem::Shutdown();

	mPlatform.Destroy();

	mTileSetHandle = InvalidTileSetHandle;
	mTileMapHandle = InvalidTileMapHandle;
	mTextureHandle = InvalidTextureHandle;
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

		Render();
	}
}

void Game::Render() {
	Renderer2D::BeginScene(mCamera);

	Renderer2D::DrawQuad({ 100.0f, 100.0f, 0.0f }, { 100.0f, 100.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });

	Renderer2D::EndScene();

	mPlatform.renderDevice->Present();
}
