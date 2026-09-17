#include "OpenGL/GLRenderDevice.h"

#include "IWindow.h"
#include "SDL3/SDL_error.h"
#include "Utils/Log.h"

#include "glad/glad.h"

GLRenderDevice::~GLRenderDevice() {
	Shutdown();
}

bool GLRenderDevice::Init(IWindow* window) {
	mWindow = static_cast<SDL_Window*>(window->GetHandle());
	mContext = SDL_GL_CreateContext(mWindow);
	if (!mContext) {
		LOG_ERROR("Failed to create OpenGL context: {}", SDL_GetError());
		return false;
	}

	if (!SDL_GL_MakeCurrent(mWindow, mContext)) {
		LOG_ERROR("Failed to make OpenGL context current: {}", SDL_GetError());
		return false;
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		LOG_ERROR("Failed to initialize OpenGL loader");
		return false;
	}

	return true;
}

void GLRenderDevice::Shutdown() {
	if (mContext) {
		SDL_GL_DestroyContext(mContext);
		mContext = nullptr;
	}

	mWindow = nullptr;
}

void GLRenderDevice::EnableVsync(bool enable) {
	if (!SDL_GL_SetSwapInterval(enable ? 1 : 0)) {
		LOG_ERROR("Failed to set Vsync: {}", SDL_GetError());
	}
}

void GLRenderDevice::Present() {
	if (!SDL_GL_SwapWindow(mWindow)) {
		LOG_ERROR("Failed to swap window: {}", SDL_GetError());
	}
}
