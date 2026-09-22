#include "OpenGL/GLRenderDevice.h"

#include "IWindow.h"
#include "SDL3/SDL_error.h"
#include "Utils/Log.h"

#include "glad/gl.h"

static void GLAPIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam);

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

	if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
		LOG_ERROR("Failed to initialize OpenGL loader");
		return false;
	}

#ifdef DEBUG
	GLint glMajorVer = 0, glMinorVer = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &glMajorVer);
	glGetIntegerv(GL_MINOR_VERSION, &glMinorVer);
	LOG_INFO("OpenGL Info:");
	LOG_INFO("  Vendor: {0}", (char*)glGetString(GL_VENDOR));
	LOG_INFO("  Renderer: {0}", (char*)glGetString(GL_RENDERER));
	LOG_INFO("  Version: {0}.{1}", glMajorVer, glMinorVer);

	if (GLAD_GL_KHR_debug) {
		LOG_INFO("Setting up OpenGL debug callback...");
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugCallback, nullptr);
	} else {
		LOG_INFO("Unable to use OpenGL debug callback");
	}
#endif

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

ITexture* GLRenderDevice::CreateTexture() {
	// Implement texture creation for OpenGL here
	return nullptr;
}

static void GLAPIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam
) {
	const char* srcStr = "Unknown";
	switch (source) {
	case GL_DEBUG_SOURCE_API:
		srcStr = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		srcStr = "Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		srcStr = "Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		srcStr = "Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		srcStr = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		srcStr = "Other";
		break;
	}

	const char* typeStr = "Unknown";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		typeStr = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typeStr = "Deprecated Behavior";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typeStr = "Undefined Behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		typeStr = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		typeStr = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		typeStr = "Marker";
		break;
	case GL_DEBUG_TYPE_OTHER:
		typeStr = "Other";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		LOG_CRITICAL("OpenGL Debug: Source: {}, Type: {}, Message: {}", srcStr, typeStr, message);
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_ERROR("OpenGL Debug: Source: {}, Type: {}, Message: {}", srcStr, typeStr, message);
		break;

	case GL_DEBUG_SEVERITY_LOW:
		LOG_WARN("OpenGL Debug: Source: {}, Type: {}, Message: {}", srcStr, typeStr, message);
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		LOG_INFO("OpenGL Debug: Source: {}, Type: {}, Message: {}", srcStr, typeStr, message);
		break;

	default:
		LOG_TRACE("OpenGL Debug: Source: {}, Type: {}, Message: {}", srcStr, typeStr, message);
		break;
	}
}
