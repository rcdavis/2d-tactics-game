#pragma once

#include "glm/ext/matrix_float4x4.hpp"

class IShader {
public:
	virtual ~IShader() = default;

	virtual bool Init(const char* const vertexFilepath, const char* const fragmentFilepath) = 0;
	virtual void Destroy() = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual void SetMat4(const char* const name, const glm::mat4& value) = 0;
};
