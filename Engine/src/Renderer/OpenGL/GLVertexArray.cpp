#include "Renderer/OpenGL/GLVertexArray.h"

#include "Renderer/Buffer.h"
#include "Renderer/OpenGL/GLBuffer.h"

#include "glad/gl.h"

namespace {
	GLenum GetGLType(BufferElementType type) {
		switch (type) {
			case BufferElementType::Float:
			case BufferElementType::Float2:
			case BufferElementType::Float3:
			case BufferElementType::Float4:
			case BufferElementType::Mat3:
			case BufferElementType::Mat4:
				return GL_FLOAT;
			case BufferElementType::Int:
			case BufferElementType::Int2:
			case BufferElementType::Int3:
			case BufferElementType::Int4:
				return GL_INT;
			case BufferElementType::Bool:
				return GL_UNSIGNED_BYTE;
			default: return GL_FLOAT;
		}
	}
}

GLVertexArray::~GLVertexArray() {
	Destroy();
}

bool GLVertexArray::Init() {
	glCreateVertexArrays(1, &mId);

	return true;
}

void GLVertexArray::Destroy() {
	if (mId) {
		glDeleteVertexArrays(1, &mId);
		mId = 0;
	}

	mVertexBuffer = nullptr;
	mIndexBuffer = nullptr;
}

void GLVertexArray::Bind() const {
	glBindVertexArray(mId);
}

void GLVertexArray::Unbind() const {
	glBindVertexArray(0);
}

void GLVertexArray::SetVertexBuffer(const IVertexBuffer* vertexBuffer) {
	mVertexBuffer = vertexBuffer;

	const GLVertexBuffer* const glVertexBuffer = static_cast<const GLVertexBuffer*>(vertexBuffer);
	const BufferLayout& layout = vertexBuffer->GetLayout();
	glVertexArrayVertexBuffer(mId, 0, glVertexBuffer->GetId(), 0, layout.GetStride());
	glVertexArrayBindingDivisor(mId, 0, 0);

	uint32_t attributeIndex = 0;
	for (const BufferElement& element : layout) {
		switch (element.type) {
		case BufferElementType::Mat3:
		case BufferElementType::Mat4: {
			const uint32_t componentCount = element.GetComponentCount();
			for (uint32_t i = 0; i < componentCount; ++i) {
				glEnableVertexArrayAttrib(mId, attributeIndex);
				glVertexArrayAttribBinding(mId, attributeIndex, 0);
				glVertexArrayAttribFormat(
					mId,
					attributeIndex,
					componentCount,
					GetGLType(element.type),
					element.normalized ? GL_TRUE : GL_FALSE,
					element.offset + sizeof(float) * componentCount * i);
				++attributeIndex;
			}
		}
		break;

		case BufferElementType::Float:
		case BufferElementType::Float2:
		case BufferElementType::Float3:
		case BufferElementType::Float4: {
			glEnableVertexArrayAttrib(mId, attributeIndex);
			glVertexArrayAttribBinding(mId, attributeIndex, 0);
			glVertexArrayAttribFormat(
				mId,
				attributeIndex,
				element.GetComponentCount(),
				GetGLType(element.type),
				element.normalized ? GL_TRUE : GL_FALSE,
				element.offset);
			++attributeIndex;
		}
		break;

		default: {
			glEnableVertexArrayAttrib(mId, attributeIndex);
			glVertexArrayAttribBinding(mId, attributeIndex, 0);
			glVertexArrayAttribIFormat(
				mId,
				attributeIndex,
				element.GetComponentCount(),
				GetGLType(element.type),
				element.offset);
			++attributeIndex;
		}
		break;
		}
	}
}

void GLVertexArray::SetIndexBuffer(const IIndexBuffer* indexBuffer) {
	mIndexBuffer = indexBuffer;

	const GLIndexBuffer* const glIndexBuffer = static_cast<const GLIndexBuffer*>(indexBuffer);
	glVertexArrayElementBuffer(mId, glIndexBuffer->GetId());
}

uint32_t GLVertexArray::GetIndexCount() const {
	return mIndexBuffer ? mIndexBuffer->GetCount() : 0;
}
