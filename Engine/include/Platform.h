#pragma once

class IRenderDevice;
class IWindow;
struct PlatformEvent;
struct WindowCreateInfo;

struct Platform {
	IRenderDevice* renderDevice = nullptr;
	IWindow* window = nullptr;

	bool Init(const WindowCreateInfo& info);

	void Destroy();

	bool PollEvent(PlatformEvent& event);
};
