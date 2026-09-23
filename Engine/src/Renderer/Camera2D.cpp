#include "Renderer/Camera2D.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

void Camera2D::SetProjection(float left, float right, float bottom, float top) {
	proj = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
	UpdateViewProj();
}

void Camera2D::UpdateViewProj() {
	viewProj = proj * glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}
