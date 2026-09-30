#pragma once

#include "Renderer/IRenderDevice.h"

#include "SDL3/SDL_video.h"

class GLRenderDevice : public IRenderDevice {
public:
	virtual ~GLRenderDevice();
	virtual bool Init(IWindow* window) override;
	virtual void Shutdown() override;

	virtual void EnableVsync(bool enable) override;

	virtual void DrawIndexed(IVertexArray* vertexArray, uint32_t indexCount = 0) override;

	virtual void Present() override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	virtual ITexture* CreateTexture() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	virtual IVertexBuffer* CreateVertexBuffer() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	virtual IIndexBuffer* CreateIndexBuffer() const override;

private:
	SDL_GLContext mContext = nullptr;
	SDL_Window* mWindow = nullptr;
};
