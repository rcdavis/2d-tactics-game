#pragma once

class IWindow;
struct PlatformEvent;
struct WindowCreateInfo;

struct Platform {
	IWindow* window = nullptr;

	bool Init(const WindowCreateInfo& info);

	void Destroy();

	bool PollEvent(PlatformEvent& event);
};
