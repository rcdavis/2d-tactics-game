#pragma once

#include "Renderer/Buffer.h"

#include <volk.h>

class VulkanRenderDevice;

class VulkanVertexBuffer final : public IVertexBuffer {
public:
	explicit VulkanVertexBuffer(VulkanRenderDevice* device = nullptr);
	~VulkanVertexBuffer() override;

	bool Init(BufferUsage usage, const BufferLayout& layout, const void* data = nullptr,
		uint32_t size = 0) override;
	void Destroy() override;
	void Bind() const override {}
	void Unbind() const override {}
	const BufferLayout& GetLayout() const override { return mLayout; }
	void SetData(const void* data, uint32_t size) override;

	VkBuffer GetBuffer() const { return mBuffer; }

private:
	VulkanRenderDevice* mDevice = nullptr;
	VkBuffer mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mMemory = VK_NULL_HANDLE;
	VkDeviceSize mSize = 0;
	BufferLayout mLayout;
};

class VulkanIndexBuffer final : public IIndexBuffer {
public:
	explicit VulkanIndexBuffer(VulkanRenderDevice* device = nullptr);
	~VulkanIndexBuffer() override;

	bool Init(BufferUsage usage, const uint16_t* data = nullptr, uint32_t count = 0) override;
	void Destroy() override;
	void Bind() const override {}
	void Unbind() const override {}
	uint32_t GetCount() const override { return mCount; }

	VkBuffer GetBuffer() const { return mBuffer; }

private:
	VulkanRenderDevice* mDevice = nullptr;
	VkBuffer mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mMemory = VK_NULL_HANDLE;
	uint32_t mCount = 0;
};
