#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Renderer/GraphicsAPI.h"

enum class ShaderDataType : uint8_t {
	Float, Float2, Float3, Float4,
	Mat3, Mat4,
	Int, Int2, Int3, Int4,
	Bool,
	Count
};

enum class BufferUsage : uint8_t {
	Static,
	Dynamic,
	Count
};

uint8_t ShaderDataTypeSize(ShaderDataType type);

struct BufferElement {
	std::string name;
	uint32_t size = 0;
	uint32_t offset = 0;
	ShaderDataType type = ShaderDataType::Float;
	bool normalized = false;

	BufferElement(const std::string& name, ShaderDataType type, bool normalized = false)
		: name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized) {}

	uint8_t GetComponentCount() const;
};

class BufferLayout {
public:
	BufferLayout() = default;
	BufferLayout(const std::initializer_list<BufferElement>& elements)
		: mElements(elements
	) {
		CalculateOffsetsAndStride();
	}

	const std::vector<BufferElement>& GetElements() const { return mElements; }
	uint32_t GetStride() const { return mStride; }

	auto begin() { return std::begin(mElements); }
	auto begin() const { return std::begin(mElements); }
	auto end() { return std::end(mElements); }
	auto end() const { return std::end(mElements); }

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

	virtual bool Init(BufferUsage usage, const BufferLayout& layout, const void* data = nullptr, uint32_t size = 0) = 0;
	virtual void Destroy() = 0;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual const BufferLayout& GetLayout() const = 0;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	static IVertexBuffer* Create(GraphicsAPI api);
};

class IIndexBuffer {
public:
	virtual ~IIndexBuffer() = default;

	virtual bool Init(BufferUsage usage, const uint16_t* data = nullptr, uint32_t count = 0) = 0;
	virtual void Destroy() = 0;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual uint32_t GetCount() const = 0;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	static IIndexBuffer* Create(GraphicsAPI api);
};
