#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Renderer/GraphicsAPI.h"

enum ShaderDataType : uint8_t {
	Float, Float2, Float3, Float4,
	Mat3, Mat4,
	Int, Int2, Int3, Int4,
	Bool,
	Count
};

uint8_t ShaderDataTypeSize(ShaderDataType type);

struct BufferElement {
	std::string name;
	uint32_t size;
	uint32_t offset;
	ShaderDataType type;
	bool normalized;

	BufferElement(const std::string& name, ShaderDataType type, bool normalized = false)
		: name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized) {}

	uint8_t GetComponentCount() const;
};

class BufferLayout {
public:
	BufferLayout(const std::initializer_list<BufferElement>& elements)
		: mElements(elements
	) {
		CalculateOffsetsAndStride();
	}

	const std::vector<BufferElement>& GetElements() const { return mElements; }
	uint32_t GetStride() const { return mStride; }

private:
	void CalculateOffsetsAndStride() {
		mStride = 0;
		for (auto& element : mElements) {
			element.offset = mStride;
			mStride += element.size;
		}
	}

	std::vector<BufferElement> mElements;
	uint32_t mStride = 0;
};

class IVertexBuffer {
public:
	virtual ~IVertexBuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual uint32_t GetCount() const = 0;
};

class IIndexBuffer {
public:
	virtual ~IIndexBuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual uint32_t GetCount() const = 0;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	static IIndexBuffer* Create(GraphicsAPI api);
};
