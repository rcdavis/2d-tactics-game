#pragma once

#include "Renderer/ITexture.h"

#include <volk.h>

class VulkanRenderDevice;

class VulkanTexture final : public ITexture {
public:
	explicit VulkanTexture(VulkanRenderDevice* device = nullptr);
	~VulkanTexture() override;

	bool Init(const char* filepath) override;
	bool Init(const void* data, uint16_t width, uint16_t height) override;
	void Destroy() override;
	void Bind(uint32_t slot = 0) override;
	uint16_t GetWidth() const override { return mWidth; }
	uint16_t GetHeight() const override { return mHeight; }

	VkImageView GetImageView() const { return mImageView; }
	VkSampler GetSampler() const { return mSampler; }

private:
	bool InitPixels(const void* data, uint16_t width, uint16_t height, uint32_t channels);
	VulkanRenderDevice* mDevice = nullptr;
	VkImage mImage = VK_NULL_HANDLE;
	VkDeviceMemory mMemory = VK_NULL_HANDLE;
	VkImageView mImageView = VK_NULL_HANDLE;
	VkSampler mSampler = VK_NULL_HANDLE;
	uint16_t mWidth = 0;
	uint16_t mHeight = 0;
};
