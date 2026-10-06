#pragma once

#include "Renderer/ITexture.h"

class GLTexture : public ITexture {
public:
    GLTexture() = default;
    ~GLTexture() override;

	bool Init(const char* const filepath) override;
	bool Init(const void* const data, uint16_t width, uint16_t height) override;

	void Destroy() override;

	void Bind(uint32_t slot = 0) override;

	uint16_t GetWidth() const override;
	uint16_t GetHeight() const override;

private:
    uint32_t mId = 0;
	uint16_t mWidth = 0;
	uint16_t mHeight = 0;
};
