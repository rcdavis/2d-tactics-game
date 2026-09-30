#pragma once

class IShader {
public:
	virtual ~IShader() = default;

	virtual bool Init(const char* const vertexFilepath, const char* const fragmentFilepath) = 0;
	virtual void Destroy() = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};
