#include "Renderer/Buffer.h"

uint8_t ShaderDataTypeSize(ShaderDataType type) {
	switch (type) {
		case Float:   return 4;
		case Float2:  return 4 * 2;
		case Float3:  return 4 * 3;
		case Float4:  return 4 * 4;
		case Mat3:    return 4 * 3 * 3;
		case Mat4:    return 4 * 4 * 4;
		case Int:     return 4;
		case Int2:    return 4 * 2;
		case Int3:    return 4 * 3;
		case Int4:    return 4 * 4;
		case Bool:    return 1;
		default:      return 0;
	}
}

uint8_t BufferElement::GetComponentCount() const {
	switch (type) {
		case Float:   return 1;
		case Float2:  return 2;
		case Float3:  return 3;
		case Float4:  return 4;
		case Mat3:    return 3;
		case Mat4:    return 4;
		case Int:     return 1;
		case Int2:    return 2;
		case Int3:    return 3;
		case Int4:    return 4;
		case Bool:    return 1;
		default:      return 0;
	}
}

IIndexBuffer* IIndexBuffer::Create(GraphicsAPI api) {
	// TODO: Add OpenGL Index buffer creation logic here
	return nullptr;
}
