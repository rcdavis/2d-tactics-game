#pragma once

#include "Renderer/Buffer.h"

class GLIndexBuffer : public IIndexBuffer {
public:
	GLIndexBuffer() = default;
	virtual ~GLIndexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual uint32_t GetCount() const override;

private:
	uint32_t mRendererID = 0;
	uint32_t mCount = 0;
};
