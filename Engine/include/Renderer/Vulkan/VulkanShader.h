#pragma once

#include "Renderer/IShader.h"

#include <volk.h>

class VulkanRenderDevice;

class VulkanShader final : public IShader {
public:
	explicit VulkanShader(VulkanRenderDevice* device = nullptr);
	~VulkanShader() override;

	bool Init(const char* vertexFilepath, const char* fragmentFilepath) override;
	bool Init(const char* const spvFilepath) override;
	void Destroy() override;
	void Bind() override {}
	void Unbind() override {}
	void SetMat4(const char* name, const glm::mat4& value) override;

	VkShaderModule GetVertexModule() const { return mVertexModule; }
	VkShaderModule GetFragmentModule() const { return mFragmentModule; }

private:
	bool LoadModule(const char* filepath, VkShaderModule& module);
	VulkanRenderDevice* mDevice = nullptr;
	VkShaderModule mVertexModule = VK_NULL_HANDLE;
	VkShaderModule mFragmentModule = VK_NULL_HANDLE;
};
