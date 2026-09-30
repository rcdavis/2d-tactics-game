#include "Renderer/OpenGL/GLBuffer.h"

#include "glad/gl.h"

namespace {
	GLenum GetGLUsage(BufferUsage usage) {
		switch (usage) {
			case BufferUsage::Static: return GL_STATIC_DRAW;
			case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
			default: return GL_STATIC_DRAW;
		}
	}
}

GLVertexBuffer::~GLVertexBuffer() {
	Destroy();
}

bool GLVertexBuffer::Init(BufferUsage usage, const BufferLayout& layout, const void* data, uint32_t size) {
	glCreateBuffers(1, &mId);
	glNamedBufferData(mId, size, data, GetGLUsage(usage));

	mLayout = layout;

	return true;
}

void GLVertexBuffer::Destroy() {
	if (mId) {
		glDeleteBuffers(1, &mId);
		mId = 0;
	}
}

void GLVertexBuffer::Bind() const {
	// Vertex Array with DSA makes this not needed
}

void GLVertexBuffer::Unbind() const {
	// Vertex Array with DSA makes this not needed
}

const BufferLayout& GLVertexBuffer::GetLayout() const {
	return mLayout;
}

GLIndexBuffer::~GLIndexBuffer() {
	Destroy();
}

bool GLIndexBuffer::Init(BufferUsage usage, const uint16_t* data, uint32_t count) {
	glCreateBuffers(1, &mId);
	glNamedBufferData(mId, count * sizeof(uint16_t), data, GetGLUsage(usage));

	mCount = count;

	return true;
}

void GLIndexBuffer::Destroy() {
	if (mId) {
		glDeleteBuffers(1, &mId);
		mId = 0;
	}

	mCount = 0;
}

void GLIndexBuffer::Bind() const {
	// Vertex Array with DSA makes this not needed
}

void GLIndexBuffer::Unbind() const {
	// Vertex Array with DSA makes this not needed
}

uint32_t GLIndexBuffer::GetCount() const {
	return mCount;
}
