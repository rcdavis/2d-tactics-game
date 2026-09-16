#include "Platform.h"

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "Utils/Log.h"

#include "PlatformEvent.h"
#include "SDL/SDLWindow.h"
#include "OpenGL/GLRenderDevice.h"

bool Platform::Init(const WindowCreateInfo& info) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		LOG_ERROR("Failed to initialize SDL3: {}", SDL_GetError());
		return false;
	}

	// TODO: Support other graphics APIs besides OpenGL and SDL
	window = new SDLWindow();
	if (!window->Init(info)) {
		LOG_ERROR("Failed to initialize window");
		delete window;
		window = nullptr;
		return false;
	}

	renderDevice = new GLRenderDevice();
	if (!renderDevice->Init(window)) {
		LOG_ERROR("Failed to initialize render device");
		delete renderDevice;
		renderDevice = nullptr;
		return false;
	}

	if (info.useVSync) {
		SDL_GL_SetSwapInterval(1);
	} else {
		SDL_GL_SetSwapInterval(0);
	}

	return true;
}

void Platform::Destroy() {
	if (renderDevice) {
		delete renderDevice;
		renderDevice = nullptr;
	}

	if (window) {
		delete window;
		window = nullptr;
	}

	SDL_Quit();
}

bool Platform::PollEvent(PlatformEvent& event) {
	SDL_Event sdlEvent {};
	if (SDL_PollEvent(&sdlEvent)) {
		switch (sdlEvent.type) {
		case SDL_EVENT_QUIT:
			event.type = PlatformEvent::Type::Quit;
			break;
		default:
			event.type = PlatformEvent::Type::Count;
			break;
		}

		return true;
	}

	return false;
}
