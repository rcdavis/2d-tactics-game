#pragma once

#include "Renderer/GraphicsAPI.h"

#include <filesystem>

class IRenderDevice;
class IWindow;
struct PlatformEvent;
struct WindowCreateInfo;

struct Platform {
	IRenderDevice* renderDevice = nullptr;
	IWindow* window = nullptr;
	GraphicsAPI graphicsApi = GraphicsAPI::OpenGL;

	bool Init(const WindowCreateInfo& info);

	void Destroy();

	bool PollEvent(PlatformEvent& event);

	std::filesystem::path GetPreferencesPath(const char* const org, const char* const app) const;
};
