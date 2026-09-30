#pragma once

class IShader {
public:
	virtual ~IShader() = default;

	virtual bool Init(const char* const vertexSource, const char* const fragmentSource) = 0;
	virtual void Destroy() = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};
