#include "Renderer/IVertexArray.h"

#include "Renderer/OpenGL/GLVertexArray.h"

IVertexArray* IVertexArray::Create(GraphicsAPI api) {
	switch (api) {
	case GraphicsAPI::OpenGL: return new GLVertexArray();
	default: return nullptr;
	}
}
