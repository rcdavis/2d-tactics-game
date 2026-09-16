#pragma once

#include "GraphicsAPI.h"

struct WindowCreateInfo {
	const char* title = nullptr;
	uint16_t width = 0;
	uint16_t height = 0;
	GraphicsAPI graphicsAPI = GraphicsAPI::OpenGL;
	bool isResizable = false;
	bool isFullscreen = false;
};

class IWindow {
public:
	virtual ~IWindow() = default;

	virtual bool Init(const WindowCreateInfo& info) = 0;
	virtual void Destroy() = 0;

	virtual void* GetHandle() const = 0;
	virtual uint16_t GetWidth() const = 0;
	virtual uint16_t GetHeight() const = 0;
};
