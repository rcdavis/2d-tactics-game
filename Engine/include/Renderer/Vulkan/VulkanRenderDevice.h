#pragma once

#include "Renderer/IRenderDevice.h"

class VulkanRenderDevice : public IRenderDevice {
public:
    VulkanRenderDevice() = default;
    virtual ~VulkanRenderDevice();

	bool Init(IWindow* window) override;
	void Shutdown() override;

	void EnableVsync(bool enable) override;
	void EnableBlending(bool enable) override;
	void EnableDepthTest(bool enable) override;

	void SetViewport(int x, int y, int width, int height) override;
	void SetClearColor(float r, float g, float b, float a) override;
	void Clear(ClearFlags flags) override;

	void DrawIndexed(
		IVertexArray* vertexArray,
		uint32_t indexCount = 0,
		DrawIndexSize size = DrawIndexSize::UShort) override;

	void Present() override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	ITexture* CreateTexture() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IShader* CreateShader() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IVertexArray* CreateVertexArray() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IVertexBuffer* CreateVertexBuffer() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IIndexBuffer* CreateIndexBuffer() const override;
};
