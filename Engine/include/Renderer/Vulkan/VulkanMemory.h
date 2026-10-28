#pragma once

#include <volk.h>

namespace VulkanMemory {
	inline uint32_t FindType(VkPhysicalDevice device, uint32_t bits,
		VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memory{};
		vkGetPhysicalDeviceMemoryProperties(device, &memory);
		for (uint32_t i = 0; i < memory.memoryTypeCount; ++i) {
			if ((bits & (1u << i)) &&
				(memory.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		return UINT32_MAX;
	}

	inline bool CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
		VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& memory) {
		VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		info.size = size;
		info.usage = usage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(device, &info, nullptr, &buffer) != VK_SUCCESS)
			return false;
		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(device, buffer, &requirements);
		const uint32_t type = FindType(physicalDevice, requirements.memoryTypeBits, properties);
		if (type == UINT32_MAX) {
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
			return false;
		}
		VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		allocation.allocationSize = requirements.size;
		allocation.memoryTypeIndex = type;
		if (vkAllocateMemory(device, &allocation, nullptr, &memory) != VK_SUCCESS) {
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
			return false;
		}
		return vkBindBufferMemory(device, buffer, memory, 0) == VK_SUCCESS;
	}
}
