#pragma once

#include "Renderer/Buffer.h"

class GLVertexBuffer : public IVertexBuffer {
public:
	GLVertexBuffer() = default;
	virtual ~GLVertexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual uint32_t GetCount() const override;

private:
	uint32_t mId = 0;
	uint32_t mCount = 0;
};

class GLIndexBuffer : public IIndexBuffer {
public:
	GLIndexBuffer() = default;
	virtual ~GLIndexBuffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual uint32_t GetCount() const override;

private:
	uint32_t mId = 0;
	uint32_t mCount = 0;
};
