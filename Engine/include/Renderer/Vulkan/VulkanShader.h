#pragma once

#include "Renderer/IShader.h"

#include <volk.h>

#include "glm/ext/matrix_float4x4.hpp"

class VulkanRenderDevice;

class VulkanShader final : public IShader {
public:
	explicit VulkanShader(VulkanRenderDevice* device = nullptr);
	~VulkanShader() override;

	bool Init(const char* vertexFilepath, const char* fragmentFilepath) override;
	bool Init(const char* const spvFilepath) override;
	void Destroy() override;
	void Bind() override;
	void Unbind() override;
	void SetMat4(const char* name, const glm::mat4& value) override;

	VkShaderModule GetVertexModule() const { return mVertexModule; }
	VkShaderModule GetFragmentModule() const { return mFragmentModule; }
	const glm::mat4& GetViewProjection() const { return mViewProjection; }

private:
	bool LoadModule(const char* filepath, VkShaderModule& module);
	VulkanRenderDevice* mDevice = nullptr;
	VkShaderModule mVertexModule = VK_NULL_HANDLE;
	VkShaderModule mFragmentModule = VK_NULL_HANDLE;
	// Stored transposed: the Slang shader reads the matrix as row-major
	glm::mat4 mViewProjection {1.0f};
};
