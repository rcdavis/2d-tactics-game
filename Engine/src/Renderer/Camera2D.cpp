#include "Renderer/Camera2D.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

void Camera2D::Init(uint16_t width, uint16_t height, Origin origin) {
	this->width = width;
	this->height = height;

	if (origin == Origin::TopLeft)
		proj = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
	else if (origin == Origin::BottomLeft)
		proj = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
}

void Camera2D::UpdateViewProj() {
	viewProj = proj * glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}
