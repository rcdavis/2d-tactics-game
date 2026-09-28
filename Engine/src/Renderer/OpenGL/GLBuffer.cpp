#include "Renderer/OpenGL/GLBuffer.h"

#include "glad/gl.h"

GLIndexBuffer::~GLIndexBuffer() {
	// Destructor implementation
}

void GLIndexBuffer::Bind() const {
	// Bind implementation
}

void GLIndexBuffer::Unbind() const {
	// Unbind implementation
}

uint32_t GLIndexBuffer::GetCount() const {
	return mCount;
}
