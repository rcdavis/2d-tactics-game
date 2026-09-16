#pragma once

#include "Window.h"

struct PlatformEvent;

struct Platform {
	Window window;

	bool Init(const WindowDesc& desc);

	void Destroy();

	bool PollEvent(PlatformEvent& event);
};
