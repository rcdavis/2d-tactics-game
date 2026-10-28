#include "Renderer/Vulkan/VulkanShader.h"

#include "Renderer/Vulkan/VulkanRenderDevice.h"
#include "Utils/Log.h"

#include <fstream>
#include <vector>

VulkanShader::VulkanShader(VulkanRenderDevice* device) : mDevice(device) {}
VulkanShader::~VulkanShader() { Destroy(); }

bool VulkanShader::LoadModule(const char* filepath, VkShaderModule& module) {
	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	if (!file) {
		LOG_ERROR("Failed to open Vulkan SPIR-V shader \"{}\"", filepath);
		return false;
	}
	const std::streamsize size = file.tellg();
	if (size <= 0 || size % sizeof(uint32_t) != 0) {
		LOG_ERROR("Vulkan shader \"{}\" is not valid SPIR-V", filepath);
		return false;
	}
	std::vector<char> code(static_cast<size_t>(size));
	file.seekg(0);
	file.read(code.data(), size);
	VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	info.codeSize = code.size();
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());
	if (vkCreateShaderModule(mDevice->GetDevice(), &info, nullptr, &module) != VK_SUCCESS) {
		LOG_ERROR("Failed to create Vulkan shader module \"{}\"", filepath);
		return false;
	}
	return true;
}

bool VulkanShader::Init(const char* vertexFilepath, const char* fragmentFilepath) {
	Destroy();
	if (!LoadModule(vertexFilepath, mVertexModule))
		return false;
	if (!LoadModule(fragmentFilepath, mFragmentModule)) {
		Destroy();
		return false;
	}
	return true;
}

void VulkanShader::Destroy() {
	if (!mDevice)
		return;
	if (mVertexModule) vkDestroyShaderModule(mDevice->GetDevice(), mVertexModule, nullptr);
	if (mFragmentModule) vkDestroyShaderModule(mDevice->GetDevice(), mFragmentModule, nullptr);
	mVertexModule = VK_NULL_HANDLE;
	mFragmentModule = VK_NULL_HANDLE;
}

void VulkanShader::SetMat4(const char* name, const glm::mat4& value) {
	(void)name;
	(void)value;
}
