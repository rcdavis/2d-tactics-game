#include "OpenGL/GLTexture.h"

#include "glad/gl.h"
#include "stb_image.h"
#include "Utils/Log.h"

#include <memory>

GLTexture::~GLTexture() {
    Destroy();
}

bool GLTexture::Init(const char* filepath) {
	const auto stbiDeleter = [](stbi_uc* data) {
		stbi_image_free(data);
	};

	int32_t width, height, channels;
	stbi_set_flip_vertically_on_load(1);
	std::unique_ptr<stbi_uc, decltype(stbiDeleter)> data(
		stbi_load(filepath, &width, &height, &channels, 0),
		stbiDeleter
	);
	if (!data) {
		LOG_ERROR("Failed to load image \"{0}\"", filepath);
		return false;
	}

	mWidth = width;
	mHeight = height;

	GLenum internalFormat = 0;
	GLenum dataFormat = 0;

	if (channels == 4) {
		internalFormat = GL_RGBA8;
		dataFormat = GL_RGBA;
	} else if (channels == 3) {
		internalFormat = GL_RGB8;
		dataFormat = GL_RGB;
	}

	if (internalFormat == 0 || dataFormat == 0) {
		LOG_ERROR("Unsupported image format for \"{0}\"", filepath);
		return false;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &mId);
	glTextureStorage2D(mId, 1, internalFormat, mWidth, mHeight);
	glTextureSubImage2D(mId, 0, 0, 0, mWidth, mHeight, dataFormat, GL_UNSIGNED_BYTE, data.get());
	glGenerateTextureMipmap(mId);

	glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return true;
}

void GLTexture::Destroy() {
    if (mId != 0) {
        glDeleteTextures(1, &mId);
        mId = 0;
    }

	mWidth = 0;
	mHeight = 0;
}

void GLTexture::Bind(uint32_t slot) {
	glBindTextureUnit(slot, mId);
}

void GLTexture::Unbind() {
	glBindTextureUnit(0, 0);
}

uint16_t GLTexture::GetWidth() const {
    return mWidth;
}

uint16_t GLTexture::GetHeight() const {
    return mHeight;
}
