#pragma once

class ITexture {
public:
	virtual ~ITexture() = default;

	virtual bool Init(const char* filePath) = 0;
	virtual void Destroy() = 0;

	virtual void Bind() = 0;
};
