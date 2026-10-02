#pragma once

#include "Renderer/Buffer.h"

class GLVertexBuffer : public IVertexBuffer {
public:
	GLVertexBuffer() = default;
	virtual ~GLVertexBuffer() override;

	virtual bool Init(BufferUsage usage, const BufferLayout& layout, const void* data = nullptr, uint32_t size = 0) override;
	virtual void Destroy() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual const BufferLayout& GetLayout() const override;

	virtual void SetData(const void* data, uint32_t size) override;

	uint32_t GetId() const { return mId; }

private:
	uint32_t mId = 0;
	BufferLayout mLayout;
};

class GLIndexBuffer : public IIndexBuffer {
public:
	GLIndexBuffer() = default;
	virtual ~GLIndexBuffer() override;

	virtual bool Init(BufferUsage usage, const uint16_t* data = nullptr, uint32_t count = 0) override;
	virtual void Destroy() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual uint32_t GetCount() const override;

	uint32_t GetId() const { return mId; }

private:
	uint32_t mId = 0;
	uint32_t mCount = 0;
};
