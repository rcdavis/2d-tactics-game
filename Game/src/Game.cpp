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

	constexpr ShaderHandle quadShaderHandle = static_cast<ShaderHandle>(Res::Shaders::Vertex::Id::TextureQuad);

	mCamera.Init(windowCreateInfo.width, windowCreateInfo.height);

	if (!Renderer2D::Init(mPlatform.renderDevice, quadShaderHandle)) {
		LOG_ERROR("Failed to initialize Renderer2D");
		return false;
	}

	mPlatform.renderDevice->SetClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	mPlatform.renderDevice->EnableDepthTest(true);

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

			switch (event.type) {
			case PlatformEvent::Type::WindowPixelSizeChanged:
				OnResize(event.windowSize.width, event.windowSize.height);
				break;
			case PlatformEvent::Type::KeyDown:
			case PlatformEvent::Type::KeyUp:
				LOG_INFO("Key event: scancode = {}", (uint16_t)event.key.scancode);
				break;
			default:
				break;
			}
		}

		Render();
	}
}

void Game::Render() {
	Renderer2D::Clear(ClearFlags::Color | ClearFlags::Depth);

	Renderer2D::BeginScene(mCamera);

	constexpr TextureHandle selectionRingTextureHandle = static_cast<TextureHandle>(Res::Textures::Id::SelectionRing);
	constexpr TileMapHandle tileMapHandle = static_cast<TileMapHandle>(Res::Tiles::Maps::Id::Toen);

	Renderer2D::DrawTexturedQuad(
		{ 0.0f, 0.0f, 0.7f },
		{ 16.0f, 16.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		selectionRingTextureHandle);
	Renderer2D::DrawTileMap(mCamera, tileMapHandle);

	Renderer2D::EndScene();

	Renderer2D::Present();
}

void Game::OnResize(uint16_t width, uint16_t height) {
	mPlatform.renderDevice->SetViewport(0, 0, width, height);
}
