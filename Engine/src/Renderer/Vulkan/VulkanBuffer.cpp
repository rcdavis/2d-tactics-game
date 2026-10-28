#include "Renderer/Vulkan/VulkanBuffer.h"

#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanRenderDevice.h"

#include <cstring>
#include <vector>

namespace {
	bool Upload(VulkanRenderDevice* device, const void* data, VkDeviceSize size,
		VkBufferUsageFlags usage, VkBuffer& destination, VkDeviceMemory& memory) {
		VkBuffer staging = VK_NULL_HANDLE;
		VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
		if (!VulkanMemory::CreateBuffer(device->GetDevice(), device->GetPhysicalDevice(), size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingMemory))
			return false;
		void* mapped = nullptr;
		vkMapMemory(device->GetDevice(), stagingMemory, 0, size, 0, &mapped);
		std::memcpy(mapped, data, static_cast<size_t>(size));
		vkUnmapMemory(device->GetDevice(), stagingMemory);
		if (!VulkanMemory::CreateBuffer(device->GetDevice(), device->GetPhysicalDevice(), size,
			usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			destination, memory)) {
			vkDestroyBuffer(device->GetDevice(), staging, nullptr);
			vkFreeMemory(device->GetDevice(), stagingMemory, nullptr);
			return false;
		}
		const bool copied = device->ExecuteImmediate([&](VkCommandBuffer command) {
			VkBufferCopy region{0, 0, size};
			vkCmdCopyBuffer(command, staging, destination, 1, &region);
		});
		vkDestroyBuffer(device->GetDevice(), staging, nullptr);
		vkFreeMemory(device->GetDevice(), stagingMemory, nullptr);
		return copied;
	}
}

VulkanVertexBuffer::VulkanVertexBuffer(VulkanRenderDevice* device) : mDevice(device) {}
VulkanVertexBuffer::~VulkanVertexBuffer() { Destroy(); }

bool VulkanVertexBuffer::Init(BufferUsage usage, const BufferLayout& layout,
	const void* data, uint32_t size) {
	Destroy();
	mLayout = layout;
	mSize = size;
	if (!size)
		return true;
	if (data && usage == BufferUsage::Static)
		return Upload(mDevice, data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mBuffer, mMemory);
	if (!VulkanMemory::CreateBuffer(mDevice->GetDevice(), mDevice->GetPhysicalDevice(), size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, usage == BufferUsage::Dynamic ?
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT :
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mBuffer, mMemory))
		return false;
	if (data) {
		void* mapped = nullptr;
		if (vkMapMemory(mDevice->GetDevice(), mMemory, 0, size, 0, &mapped) != VK_SUCCESS)
			return false;
		std::memcpy(mapped, data, size);
		vkUnmapMemory(mDevice->GetDevice(), mMemory);
	}
	return true;
}

void VulkanVertexBuffer::Destroy() {
	if (!mDevice)
		return;
	if (mBuffer) vkDestroyBuffer(mDevice->GetDevice(), mBuffer, nullptr);
	if (mMemory) vkFreeMemory(mDevice->GetDevice(), mMemory, nullptr);
	mBuffer = VK_NULL_HANDLE;
	mMemory = VK_NULL_HANDLE;
	mSize = 0;
}

void VulkanVertexBuffer::SetData(const void* data, uint32_t size) {
	if (!mBuffer || !data || size > mSize)
		return;
	void* mapped = nullptr;
	if (vkMapMemory(mDevice->GetDevice(), mMemory, 0, size, 0, &mapped) == VK_SUCCESS) {
		std::memcpy(mapped, data, size);
		vkUnmapMemory(mDevice->GetDevice(), mMemory);
	}
}

VulkanIndexBuffer::VulkanIndexBuffer(VulkanRenderDevice* device) : mDevice(device) {}
VulkanIndexBuffer::~VulkanIndexBuffer() { Destroy(); }

bool VulkanIndexBuffer::Init(BufferUsage usage, const uint16_t* data, uint32_t count) {
	Destroy();
	mCount = count;
	if (!count)
		return true;
	const VkDeviceSize size = count * sizeof(uint16_t);
	if (data && usage == BufferUsage::Static)
		return Upload(mDevice, data, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mBuffer, mMemory);
	if (!VulkanMemory::CreateBuffer(mDevice->GetDevice(), mDevice->GetPhysicalDevice(), size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, usage == BufferUsage::Dynamic ?
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT :
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mBuffer, mMemory))
		return false;
	if (data) {
		void* mapped = nullptr;
		if (vkMapMemory(mDevice->GetDevice(), mMemory, 0, size, 0, &mapped) != VK_SUCCESS)
			return false;
		std::memcpy(mapped, data, size);
		vkUnmapMemory(mDevice->GetDevice(), mMemory);
	}
	return true;
}

void VulkanIndexBuffer::Destroy() {
	if (!mDevice)
		return;
	if (mBuffer) vkDestroyBuffer(mDevice->GetDevice(), mBuffer, nullptr);
	if (mMemory) vkFreeMemory(mDevice->GetDevice(), mMemory, nullptr);
	mBuffer = VK_NULL_HANDLE;
	mMemory = VK_NULL_HANDLE;
	mCount = 0;
}
