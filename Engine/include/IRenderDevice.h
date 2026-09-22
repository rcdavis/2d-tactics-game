#pragma once

#include "GraphicsAPI.h"

class IWindow;
class ITexture;

class IRenderDevice {
public:
	virtual ~IRenderDevice() = default;
	virtual bool Init(IWindow* window) = 0;
	virtual void Shutdown() = 0;

	virtual void EnableVsync(bool enable) = 0;

	virtual void Present() = 0;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	virtual ITexture* CreateTexture() = 0;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	static IRenderDevice* Create(GraphicsAPI api);
};
