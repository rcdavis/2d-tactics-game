#include "Renderer/Vulkan/VulkanTexture.h"

#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanRenderDevice.h"
#include "Utils/Log.h"
#include "stb_image.h"

#include <memory>
#include <cstring>
#include <cstdint>

VulkanTexture::VulkanTexture(VulkanRenderDevice* device) : mDevice(device) {}
VulkanTexture::~VulkanTexture() { Destroy(); }

bool VulkanTexture::Init(const char* filepath) {
	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_set_flip_vertically_on_load(1);
	stbi_uc* pixels = stbi_load(filepath, &width, &height, &channels, 4);
	if (!pixels) {
		LOG_ERROR("Failed to load image \"{}\"", filepath);
		return false;
	}
	if (width > UINT16_MAX || height > UINT16_MAX) {
		LOG_ERROR("Image dimensions exceed Vulkan texture interface limits: \"{}\"", filepath);
		stbi_image_free(pixels);
		return false;
	}
	const bool success = InitPixels(pixels, static_cast<uint16_t>(width),
		static_cast<uint16_t>(height), 4);
	stbi_image_free(pixels);
	return success;
}

bool VulkanTexture::Init(const void* data, uint16_t width, uint16_t height) {
	return InitPixels(data, width, height, 4);
}

bool VulkanTexture::InitPixels(const void* data, uint16_t width, uint16_t height,
	uint32_t channels) {
	if (!data || !width || !height || channels != 4)
		return false;
	Destroy();
	mWidth = width;
	mHeight = height;
	VkDevice device = mDevice->GetDevice();
	const VkDeviceSize size = static_cast<VkDeviceSize>(width) * height * 4;
	VkBuffer staging = VK_NULL_HANDLE;
	VmaAllocation stagingAllocation = VK_NULL_HANDLE;
	if (!VulkanMemory::CreateBuffer(mDevice->GetAllocator(), size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, staging,
		stagingAllocation))
		return false;
	void* mapped = nullptr;
	vmaMapMemory(mDevice->GetAllocator(), stagingAllocation, &mapped);
	std::memcpy(mapped, data, static_cast<size_t>(size));
	vmaUnmapMemory(mDevice->GetAllocator(), stagingAllocation);

	VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = {width, height, 1};
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	VmaAllocationCreateInfo imageAllocation{};
	imageAllocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(mDevice->GetAllocator(), &imageInfo, &imageAllocation,
		&mImage, &mAllocation, nullptr) != VK_SUCCESS) {
		vmaDestroyBuffer(mDevice->GetAllocator(), staging, stagingAllocation);
		return false;
	}
	const bool uploaded = mDevice->ExecuteImmediate([&](VkCommandBuffer command) {
		VkImageMemoryBarrier toTransfer{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toTransfer.image = mImage;
		toTransfer.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toTransfer);
		VkBufferImageCopy copy{};
		copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		copy.imageExtent = {width, height, 1};
		vkCmdCopyBufferToImage(command, staging, mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		toTransfer.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		toTransfer.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		toTransfer.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toTransfer.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toTransfer);
	});
	vmaDestroyBuffer(mDevice->GetAllocator(), staging, stagingAllocation);
	if (!uploaded)
		return false;

	VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	view.image = mImage;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = VK_FORMAT_R8G8B8A8_UNORM;
	view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	if (vkCreateImageView(device, &view, nullptr, &mImageView) != VK_SUCCESS)
		return false;
	VkSamplerCreateInfo sampler{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.maxLod = 1.0f;
	if (vkCreateSampler(device, &sampler, nullptr, &mSampler) != VK_SUCCESS) {
		Destroy();
		return false;
	}
	return true;
}

void VulkanTexture::Destroy() {
	if (!mDevice)
		return;
	VkDevice device = mDevice->GetDevice();
	if (mSampler) vkDestroySampler(device, mSampler, nullptr);
	if (mImageView) vkDestroyImageView(device, mImageView, nullptr);
	if (mImage) vmaDestroyImage(mDevice->GetAllocator(), mImage, mAllocation);
	mSampler = VK_NULL_HANDLE;
	mImageView = VK_NULL_HANDLE;
	mImage = VK_NULL_HANDLE;
	mAllocation = VK_NULL_HANDLE;
	mWidth = 0;
	mHeight = 0;
}

void VulkanTexture::Bind(uint32_t slot) {
	if (mDevice)
		mDevice->BindTexture(slot, this);
}
