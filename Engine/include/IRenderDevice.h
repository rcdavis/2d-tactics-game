#pragma once

class IWindow;

class IRenderDevice {
public:
	virtual ~IRenderDevice() = default;
	virtual bool Init(IWindow* window) = 0;
	virtual void Shutdown() = 0;
	virtual void Present() = 0;
};
