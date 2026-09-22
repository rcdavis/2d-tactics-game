#include "OpenGL/GLTexture.h"

#include <glad/gl.h>

GLTexture::~GLTexture() {
    Destroy();
}

bool GLTexture::Init(const char* filePath) {
    // Implementation for initializing the texture
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

void GLTexture::Bind() {
	glBindTexture(GL_TEXTURE_2D, mId);
}

void GLTexture::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

uint16_t GLTexture::GetWidth() const {
    return mWidth;
}

uint16_t GLTexture::GetHeight() const {
    return mHeight;
}
