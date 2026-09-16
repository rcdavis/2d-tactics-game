#include "Window.h"

#include "SDL3/SDL_video.h"
#include "Utils/Log.h"

Window::~Window() {
	Destroy();
}

bool Window::Init(const WindowCreateInfo& info) {
	SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL;
	if (info.isResizable)
		windowFlags |= SDL_WINDOW_RESIZABLE;
	if (info.isFullscreen)
		windowFlags |= SDL_WINDOW_FULLSCREEN;

	mHandle = SDL_CreateWindow(info.title, info.width, info.height, windowFlags);
	if (!mHandle) {
		LOG_ERROR("Failed to create SDL3 window: {}", SDL_GetError());
		return false;
	}

	mWidth = info.width;
	mHeight = info.height;

	return true;
}

void Window::Destroy() {
	if (mHandle) {
		SDL_DestroyWindow(static_cast<SDL_Window*>(mHandle));
		mHandle = nullptr;
	}

	mWidth = 0;
	mHeight = 0;
}
