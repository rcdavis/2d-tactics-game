#pragma once

#include "Renderer/IShader.h"

#include <cstdint>

class GLShader : public IShader {
public:
	GLShader() = default;
	virtual ~GLShader() override;

	virtual bool Init(const char* const vertexFilepath, const char* const fragmentFilepath) override;
	virtual void Destroy() override;

	virtual void Bind() override;
	virtual void Unbind() override;

private:
	uint32_t mId = 0;
};
