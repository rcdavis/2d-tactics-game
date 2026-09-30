#include "Renderer/OpenGL/GLShader.h"

#include "glad/gl.h"

#include "Utils/Log.h"

#include <fstream>
#include <vector>

namespace {
	uint32_t CompileShader(GLenum type, const char* const filepath) {
		std::ifstream file(filepath, std::ios::ate);
		if (!file) {
			LOG_ERROR("Failed to open shader file: {}", filepath);
			return 0;
		}

		const size_t fileSize = static_cast<size_t>(file.tellg());
		file.seekg(0);
		std::vector<GLchar> source(fileSize);
		file.read(std::data(source), fileSize);
		const GLchar* const sourceCStr = std::data(source);

		const uint32_t shader = glCreateShader(type);
		glShaderSource(shader, 1, &sourceCStr, nullptr);
		glCompileShader(shader);

		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (!isCompiled) {
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, std::data(infoLog));

			LOG_ERROR("Failed to compile shader: {}: {}", filepath, std::data(infoLog));
			glDeleteShader(shader);

			return 0;
		}

		return shader;
	}
}

GLShader::~GLShader() {
	Destroy();
}

bool GLShader::Init(const char* const vertexFilepath, const char* const fragmentFilepath) {
	// TODO: Implement shader initialization
	const uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexFilepath);
	if (!vertexShader) {
		return false;
	}

	const uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentFilepath);
	if (!fragmentShader) {
		glDeleteShader(vertexShader);
		return false;
	}

	mId = glCreateProgram();
	glAttachShader(mId, vertexShader);
	glAttachShader(mId, fragmentShader);
	glLinkProgram(mId);

	GLint isLinked = 0;
	glGetProgramiv(mId, GL_LINK_STATUS, &isLinked);
	if (!isLinked) {
		GLint maxLength = 0;
		glGetProgramiv(mId, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(mId, maxLength, &maxLength, std::data(infoLog));

		LOG_ERROR("Failed to link shader program: {}", std::data(infoLog));
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		glDeleteProgram(mId);
		mId = 0;

		return false;
	}

	glDetachShader(mId, vertexShader);
	glDetachShader(mId, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

void GLShader::Destroy() {
	if (mId) {
		glDeleteProgram(mId);
		mId = 0;
	}
}

void GLShader::Bind() {
	glUseProgram(mId);
}

void GLShader::Unbind() {
	glUseProgram(0);
}
