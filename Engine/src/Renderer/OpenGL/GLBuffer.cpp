#include "Renderer/OpenGL/GLBuffer.h"

#include "glad/gl.h"

GLVertexBuffer::~GLVertexBuffer() {
	// Destructor implementation
}

void GLVertexBuffer::Bind() const {
	// Bind implementation
}

void GLVertexBuffer::Unbind() const {
	// Unbind implementation
}

uint32_t GLVertexBuffer::GetCount() const {
	return mCount;
}

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
