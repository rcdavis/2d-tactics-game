#pragma once

#include <cstdint>

struct WindowDesc {
    const char* title = nullptr;
    uint16_t width = 0;
    uint16_t height = 0;
	bool isResizable = false;
	bool isFullscreen = false;
};

class Window {
public:
    Window() = default;
    ~Window();

	bool Init(const WindowDesc& desc);
	void Destroy();

	uint16_t GetWidth() const { return mWidth; }
	uint16_t GetHeight() const { return mHeight; }

private:
	void* mHandle = nullptr;
	uint16_t mWidth = 0;
	uint16_t mHeight = 0;
};
