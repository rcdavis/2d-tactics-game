#include "Window.h"

#include "SDL3/SDL_video.h"
#include "Utils/Log.h"

Window::~Window() {
    Destroy();
}

bool Window::Init(const WindowDesc& desc) {
	SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL;
	if (desc.isResizable)
		windowFlags |= SDL_WINDOW_RESIZABLE;
	if (desc.isFullscreen)
		windowFlags |= SDL_WINDOW_FULLSCREEN;

    mHandle = SDL_CreateWindow(desc.title, desc.width, desc.height, windowFlags);
    if (!mHandle) {
        LOG_ERROR("Failed to create SDL3 window: {}", SDL_GetError());
        return false;
    }

	mWidth = desc.width;
    mHeight = desc.height;

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
