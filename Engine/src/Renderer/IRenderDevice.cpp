#include "Renderer/IRenderDevice.h"

#include "Renderer/OpenGL/GLRenderDevice.h"
#include "Renderer/Vulkan/VulkanRenderDevice.h"

IRenderDevice* IRenderDevice::Create(GraphicsAPI api) {
	switch (api) {
	case GraphicsAPI::OpenGL:
		return new GLRenderDevice();
	case GraphicsAPI::Vulkan:
		return new VulkanRenderDevice();
	default:
		break;
	}
	return nullptr;
}
