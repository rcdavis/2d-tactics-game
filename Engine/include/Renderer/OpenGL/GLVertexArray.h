#pragma once

#include "Renderer/IVertexArray.h"

class GLVertexArray : public IVertexArray {
public:
	GLVertexArray() = default;
	virtual ~GLVertexArray() override;

	virtual bool Init() override;
	virtual void Destroy() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void SetVertexBuffer(const IVertexBuffer* vertexBuffer) override;
	virtual void SetIndexBuffer(const IIndexBuffer* indexBuffer) override;

	virtual uint32_t GetIndexCount() const override;

private:
	const IVertexBuffer* mVertexBuffer = nullptr;
	const IIndexBuffer* mIndexBuffer = nullptr;

	uint32_t mId = 0;
};
