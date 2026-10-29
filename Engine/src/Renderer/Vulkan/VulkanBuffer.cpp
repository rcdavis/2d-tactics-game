#include "Renderer/Vulkan/VulkanBuffer.h"

#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanRenderDevice.h"
#include "Utils/Log.h"

#include <cstring>
#include <vector>

namespace {
	bool Upload(VulkanRenderDevice* device, const void* data, VkDeviceSize size,
		VkBufferUsageFlags usage, VkBuffer& destination, VmaAllocation& allocation) {
		VkBuffer staging = VK_NULL_HANDLE;
		VmaAllocation stagingAllocation = VK_NULL_HANDLE;
		if (!VulkanMemory::CreateBuffer(device->GetAllocator(), size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, staging,
			stagingAllocation))
			return false;
		void* mapped = nullptr;
		vmaMapMemory(device->GetAllocator(), stagingAllocation, &mapped);
		std::memcpy(mapped, data, static_cast<size_t>(size));
		vmaUnmapMemory(device->GetAllocator(), stagingAllocation);
		if (!VulkanMemory::CreateBuffer(device->GetAllocator(), size,
			usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
			destination, allocation)) {
			vmaDestroyBuffer(device->GetAllocator(), staging, stagingAllocation);
			return false;
		}
		const bool copied = device->ExecuteImmediate([&](VkCommandBuffer command) {
			VkBufferCopy region{0, 0, size};
			vkCmdCopyBuffer(command, staging, destination, 1, &region);
		});
		vmaDestroyBuffer(device->GetAllocator(), staging, stagingAllocation);
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
	mRegionSize = size * DynamicRegions;
	mDynamic = usage == BufferUsage::Dynamic;
	if (!size)
		return true;
	if (data && usage == BufferUsage::Static)
		return Upload(mDevice, data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mBuffer, mAllocation);
	const VkDeviceSize allocationSize = mDynamic ?
		mRegionSize * VulkanRenderDevice::MaxFramesInFlight : size;
	if (!VulkanMemory::CreateBuffer(mDevice->GetAllocator(), allocationSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, mBuffer, mAllocation))
		return false;
	if (data) {
		void* mapped = nullptr;
		if (vmaMapMemory(mDevice->GetAllocator(), mAllocation, &mapped) != VK_SUCCESS)
			return false;
		std::memcpy(mapped, data, size);
		vmaUnmapMemory(mDevice->GetAllocator(), mAllocation);
	}
	return true;
}

void VulkanVertexBuffer::Destroy() {
	if (!mDevice)
		return;
	if (mBuffer) vmaDestroyBuffer(mDevice->GetAllocator(), mBuffer, mAllocation);
	mBuffer = VK_NULL_HANDLE;
	mAllocation = VK_NULL_HANDLE;
	mSize = 0;
	mCursor = 0;
	mBindOffset = 0;
}

void VulkanVertexBuffer::SetData(const void* data, uint32_t size) {
	if (!mBuffer || !data || size > mSize)
		return;
	VkDeviceSize offset = 0;
	if (mDynamic) {
		if (mFrameSerial != mDevice->GetFrameSerial()) {
			mFrameSerial = mDevice->GetFrameSerial();
			mCursor = 0;
		}
		if (mCursor + size > mRegionSize) {
			LOG_ERROR("Vulkan dynamic vertex buffer exceeded its per-frame capacity");
			return;
		}
		offset = mDevice->GetFrameIndex() * mRegionSize + mCursor;
		mCursor += (size + 15) & ~VkDeviceSize(15);
		if (mCursor > mRegionSize)
			mCursor = mRegionSize;
	}
	void* mapped = nullptr;
	if (vmaMapMemory(mDevice->GetAllocator(), mAllocation, &mapped) == VK_SUCCESS) {
		std::memcpy(static_cast<char*>(mapped) + offset, data, size);
		vmaFlushAllocation(mDevice->GetAllocator(), mAllocation, offset, size);
		vmaUnmapMemory(mDevice->GetAllocator(), mAllocation);
		mBindOffset = offset;
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
		return Upload(mDevice, data, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mBuffer, mAllocation);
	if (!VulkanMemory::CreateBuffer(mDevice->GetAllocator(), size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, mBuffer, mAllocation))
		return false;
	if (data) {
		void* mapped = nullptr;
		if (vmaMapMemory(mDevice->GetAllocator(), mAllocation, &mapped) != VK_SUCCESS)
			return false;
		std::memcpy(mapped, data, size);
		vmaUnmapMemory(mDevice->GetAllocator(), mAllocation);
	}
	return true;
}

void VulkanIndexBuffer::Destroy() {
	if (!mDevice)
		return;
	if (mBuffer) vmaDestroyBuffer(mDevice->GetAllocator(), mBuffer, mAllocation);
	mBuffer = VK_NULL_HANDLE;
	mAllocation = VK_NULL_HANDLE;
	mCount = 0;
}
