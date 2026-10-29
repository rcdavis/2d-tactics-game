#pragma once

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include <vk_mem_alloc.h>

namespace VulkanMemory {
	inline bool CreateBuffer(VmaAllocator allocator, VkDeviceSize size,
		VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
		VkBuffer& buffer, VmaAllocation& allocation) {
		VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		info.size = size;
		info.usage = usage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = memoryUsage;
		return vmaCreateBuffer(allocator, &info, &allocationInfo, &buffer,
			&allocation, nullptr) == VK_SUCCESS;
	}
}
