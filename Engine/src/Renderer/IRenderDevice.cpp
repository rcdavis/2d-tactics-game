#include "Renderer/IRenderDevice.h"

#include "Renderer/OpenGL/GLRenderDevice.h"

IRenderDevice* IRenderDevice::Create(GraphicsAPI api) {
	switch (api) {
	case GraphicsAPI::OpenGL:
		return new GLRenderDevice();
	default:
		break;
	}
	return nullptr;
}
