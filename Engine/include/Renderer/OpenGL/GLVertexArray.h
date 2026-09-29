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

	virtual void SetVertexBuffer(IVertexBuffer* vertexBuffer) override;
	virtual void SetIndexBuffer(IIndexBuffer* indexBuffer) override;

private:
	IVertexBuffer* mVertexBuffer = nullptr;
	IIndexBuffer* mIndexBuffer = nullptr;

	uint32_t mId = 0;
};
