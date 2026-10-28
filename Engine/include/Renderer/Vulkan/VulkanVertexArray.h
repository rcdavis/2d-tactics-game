#pragma once

#include "Renderer/IVertexArray.h"

class VulkanRenderDevice;

class VulkanVertexArray final : public IVertexArray {
public:
	explicit VulkanVertexArray(VulkanRenderDevice* device = nullptr);
	~VulkanVertexArray() override;

	bool Init() override;
	void Destroy() override;
	void Bind() const override;
	void Unbind() const override {}
	void SetVertexBuffer(const IVertexBuffer* vertexBuffer) override;
	void SetIndexBuffer(const IIndexBuffer* indexBuffer) override;
	uint32_t GetIndexCount() const override;

private:
	VulkanRenderDevice* mDevice = nullptr;
	const IVertexBuffer* mVertexBuffer = nullptr;
	const IIndexBuffer* mIndexBuffer = nullptr;
};
