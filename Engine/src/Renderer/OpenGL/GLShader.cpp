#include "Renderer/OpenGL/GLShader.h"

GLShader::~GLShader() {
	Destroy();
}

bool GLShader::Init(const char* const vertexSource, const char* const fragmentSource) {
	// TODO: Implement shader initialization
	return true;
}

void GLShader::Destroy() {
	// TODO: Implement shader destruction
}

void GLShader::Bind() {
	// TODO: Implement shader binding
}

void GLShader::Unbind() {
	// TODO: Implement shader unbinding
}
