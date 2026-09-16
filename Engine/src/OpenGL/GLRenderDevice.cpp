#include "OpenGL/GLRenderDevice.h"

#include "IWindow.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_error.h"
#include "Utils/Log.h"

#include "glad/glad.h"

GLRenderDevice::~GLRenderDevice() {
	Shutdown();
}

bool GLRenderDevice::Init(IWindow* window) {
	SDL_Window* sdlWindow = static_cast<SDL_Window*>(window->GetHandle());
	mContext = SDL_GL_CreateContext(sdlWindow);
	if (!mContext) {
		LOG_ERROR("Failed to create OpenGL context: {}", SDL_GetError());
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
}

void GLRenderDevice::Present() {
	// Present code here
}
