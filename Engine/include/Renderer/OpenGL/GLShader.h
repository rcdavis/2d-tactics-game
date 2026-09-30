#pragma once

#include "Renderer/IShader.h"

class GLShader : public IShader {
public:
	GLShader() = default;
	virtual ~GLShader() override;

	virtual bool Init(const char* const vertexSource, const char* const fragmentSource) override;
	virtual void Destroy() override;

	virtual void Bind() override;
	virtual void Unbind() override;
};
