#pragma once

#include "ITexture.h"

class GLTexture : public ITexture {
public:
    GLTexture() = default;
    ~GLTexture() override;

	bool Init(const char* filepath) override;
	void Destroy() override;

	void Bind(uint32_t slot = 0) override;
	void Unbind() override;

	uint16_t GetWidth() const override;
	uint16_t GetHeight() const override;

private:
    uint32_t mId = 0;
	uint16_t mWidth = 0;
	uint16_t mHeight = 0;
};
