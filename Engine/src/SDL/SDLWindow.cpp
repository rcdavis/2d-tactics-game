#include "SDL/SDLWindow.h"

#include "SDL3/SDL_video.h"
#include "Utils/Log.h"

SDLWindow::~SDLWindow() {
	Destroy();
}

bool SDLWindow::Init(const WindowCreateInfo& info) {
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

void SDLWindow::Destroy() {
	if (mHandle) {
		SDL_DestroyWindow(mHandle);
		mHandle = nullptr;
	}

	mWidth = 0;
	mHeight = 0;
}
