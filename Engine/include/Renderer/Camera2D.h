#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"

struct Camera2D {
	enum class Origin : uint8_t {
		TopLeft,
		BottomLeft,
	};

	glm::mat4 proj {1.0f};
	glm::mat4 viewProj {1.0f};
    glm::vec2 position {0.0f, 0.0f};
	uint16_t width = 0;
	uint16_t height = 0;

	void Init(uint16_t width, uint16_t height, Origin origin = Origin::TopLeft);

	void UpdateViewProj();
};
