#pragma once

#include "IWindow.h"

struct SDL_Window;

class SDLWindow : public IWindow {
public:
	SDLWindow() = default;
	~SDLWindow() override;

	bool Init(const WindowCreateInfo& info) override;
	void Destroy() override;

	void* GetHandle() const override { return mHandle; }
	uint16_t GetWidth() const override { return mWidth; }
	uint16_t GetHeight() const override { return mHeight; }

private:
	SDL_Window* mHandle = nullptr;
	uint16_t mWidth = 0;
	uint16_t mHeight = 0;
};
