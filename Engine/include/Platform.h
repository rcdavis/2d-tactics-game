#pragma once

#include "Window.h"

struct PlatformEvent;

struct Platform {
	Window window;

	bool Init(const WindowCreateInfo& info);

	void Destroy();

	bool PollEvent(PlatformEvent& event);
};
