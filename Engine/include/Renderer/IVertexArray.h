#pragma once

#include "Renderer/GraphicsAPI.h"

class IVertexBuffer;
class IIndexBuffer;

class IVertexArray {
public:
	virtual ~IVertexArray() = default;

	virtual bool Init() = 0;
	virtual void Destroy() = 0;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void SetVertexBuffer(const IVertexBuffer* vertexBuffer) = 0;
	virtual void SetIndexBuffer(const IIndexBuffer* indexBuffer) = 0;

	virtual uint32_t GetIndexCount() const = 0;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	static IVertexArray* Create(GraphicsAPI api);
};
