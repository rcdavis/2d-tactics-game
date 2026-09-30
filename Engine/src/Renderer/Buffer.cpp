#include "Renderer/Buffer.h"

#include "Renderer/OpenGL/GLBuffer.h"

uint8_t BufferElementTypeSize(BufferElementType type) {
	switch (type) {
		case BufferElementType::Float:   return 4;
		case BufferElementType::Float2:  return 4 * 2;
		case BufferElementType::Float3:  return 4 * 3;
		case BufferElementType::Float4:  return 4 * 4;
		case BufferElementType::Mat3:    return 4 * 3 * 3;
		case BufferElementType::Mat4:    return 4 * 4 * 4;
		case BufferElementType::Int:     return 4;
		case BufferElementType::Int2:    return 4 * 2;
		case BufferElementType::Int3:    return 4 * 3;
		case BufferElementType::Int4:    return 4 * 4;
		case BufferElementType::Bool:    return 1;
		default:      return 0;
	}
}

uint8_t BufferElement::GetComponentCount() const {
	switch (type) {
		case BufferElementType::Float:   return 1;
		case BufferElementType::Float2:  return 2;
		case BufferElementType::Float3:  return 3;
		case BufferElementType::Float4:  return 4;
		case BufferElementType::Mat3:    return 3;
		case BufferElementType::Mat4:    return 4;
		case BufferElementType::Int:     return 1;
		case BufferElementType::Int2:    return 2;
		case BufferElementType::Int3:    return 3;
		case BufferElementType::Int4:    return 4;
		case BufferElementType::Bool:    return 1;
		default:      return 0;
	}
}

IVertexBuffer* IVertexBuffer::Create(GraphicsAPI api) {
	switch (api) {
		case GraphicsAPI::OpenGL: return new GLVertexBuffer();
		default: return nullptr;
	}
}

IIndexBuffer* IIndexBuffer::Create(GraphicsAPI api) {
	switch (api) {
		case GraphicsAPI::OpenGL: return new GLIndexBuffer();
		default: return nullptr;
	}
}
