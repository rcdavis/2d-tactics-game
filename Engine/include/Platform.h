#pragma once

#include "Renderer/GraphicsAPI.h"

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
};
