#include "SDL/SDLWindow.h"

#include "SDL3/SDL_video.h"
#include "Utils/Log.h"

SDLWindow::~SDLWindow() {
	Destroy();
}

bool SDLWindow::Init(const WindowCreateInfo& info) {
	SDL_WindowFlags windowFlags = 0;
	if (info.graphicsAPI == GraphicsAPI::OpenGL) {
		windowFlags |= SDL_WINDOW_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	} else {
		LOG_ERROR("Unsupported graphics API. Only supporting OpenGL for now.");
		return false;
	}

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
