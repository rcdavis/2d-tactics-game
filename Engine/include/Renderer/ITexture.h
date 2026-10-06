#pragma once

#include <cstdint>

class ITexture {
public:
	virtual ~ITexture() = default;

	virtual bool Init(const char* const filepath) = 0;
	virtual bool Init(const void* const data, uint16_t width, uint16_t height) = 0;
	virtual void Destroy() = 0;

	virtual void Bind(uint32_t slot = 0) = 0;

	virtual uint16_t GetWidth() const = 0;
	virtual uint16_t GetHeight() const = 0;
};
