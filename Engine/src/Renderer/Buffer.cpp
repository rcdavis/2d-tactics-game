#include "Renderer/Buffer.h"

#include "Renderer/OpenGL/GLBuffer.h"

uint8_t ShaderDataTypeSize(ShaderDataType type) {
	switch (type) {
		case ShaderDataType::Float:   return 4;
		case ShaderDataType::Float2:  return 4 * 2;
		case ShaderDataType::Float3:  return 4 * 3;
		case ShaderDataType::Float4:  return 4 * 4;
		case ShaderDataType::Mat3:    return 4 * 3 * 3;
		case ShaderDataType::Mat4:    return 4 * 4 * 4;
		case ShaderDataType::Int:     return 4;
		case ShaderDataType::Int2:    return 4 * 2;
		case ShaderDataType::Int3:    return 4 * 3;
		case ShaderDataType::Int4:    return 4 * 4;
		case ShaderDataType::Bool:    return 1;
		default:      return 0;
	}
}

uint8_t BufferElement::GetComponentCount() const {
	switch (type) {
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Float2:  return 2;
		case ShaderDataType::Float3:  return 3;
		case ShaderDataType::Float4:  return 4;
		case ShaderDataType::Mat3:    return 3;
		case ShaderDataType::Mat4:    return 4;
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::Int2:    return 2;
		case ShaderDataType::Int3:    return 3;
		case ShaderDataType::Int4:    return 4;
		case ShaderDataType::Bool:    return 1;
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
