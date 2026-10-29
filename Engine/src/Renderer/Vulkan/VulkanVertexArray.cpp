#include "Renderer/Vulkan/VulkanVertexArray.h"

#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanRenderDevice.h"

VulkanVertexArray::VulkanVertexArray(VulkanRenderDevice* device) : mDevice(device) {}
VulkanVertexArray::~VulkanVertexArray() { Destroy(); }

bool VulkanVertexArray::Init() {
	return mDevice != nullptr;
}

void VulkanVertexArray::Destroy() {
	mVertexBuffer = nullptr;
	mIndexBuffer = nullptr;
}

void VulkanVertexArray::Bind() const {
	if (!mDevice || !mDevice->GetCurrentCommandBuffer() || !mVertexBuffer)
		return;
	const auto* vertexBuffer = static_cast<const VulkanVertexBuffer*>(mVertexBuffer);
	VkDeviceSize offset = vertexBuffer->GetBindOffset();
	const VkBuffer buffer = vertexBuffer->GetBuffer();
	vkCmdBindVertexBuffers(mDevice->GetCurrentCommandBuffer(), 0, 1,
		&buffer, &offset);
	if (mIndexBuffer) {
		const auto* indexBuffer = static_cast<const VulkanIndexBuffer*>(mIndexBuffer);
		vkCmdBindIndexBuffer(mDevice->GetCurrentCommandBuffer(), indexBuffer->GetBuffer(),
			0, VK_INDEX_TYPE_UINT16);
	}
}

void VulkanVertexArray::SetVertexBuffer(const IVertexBuffer* vertexBuffer) {
	mVertexBuffer = vertexBuffer;
}

void VulkanVertexArray::SetIndexBuffer(const IIndexBuffer* indexBuffer) {
	mIndexBuffer = indexBuffer;
}

uint32_t VulkanVertexArray::GetIndexCount() const {
	return mIndexBuffer ? mIndexBuffer->GetCount() : 0;
}
