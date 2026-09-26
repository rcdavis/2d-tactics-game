#include "Renderer/Renderer2D.h"

#include <array>
#include <cstdint>

#include "Renderer/Camera2D.h"
#include "Renderer/TextureHandle.h"

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

namespace Renderer2D {
	struct QuadVertex {
		glm::vec3 position {0.0f, 0.0f, 0.0f};
		glm::vec4 color {1.0f, 1.0f, 1.0f, 1.0f};
		glm::vec2 texCoord {0.0f, 0.0f};
		int32_t texIndex = 0;
	};

	static constexpr uint32_t MaxQuads = 1'000;
	static constexpr uint32_t MaxVertices = MaxQuads * 4;
	static constexpr uint32_t MaxIndices = MaxQuads * 6;
	static constexpr uint32_t MaxTextureSlots = 32;

	static QuadVertex* s_QuadVertexBufferData = nullptr;
	static QuadVertex* s_QuadVertexBufferCurrent = nullptr;

	static uint32_t s_QuadIndexCount = 0;

	static std::array<TextureHandle, MaxTextureSlots> s_TextureSlots {};
	static uint32_t s_TextureSlotIndex = 0;

	bool Init() {
		s_QuadVertexBufferData = new QuadVertex[MaxVertices];
		s_QuadVertexBufferCurrent = s_QuadVertexBufferData;

		s_TextureSlots.fill(InvalidTextureHandle);
		s_TextureSlotIndex = 0;

		s_QuadIndexCount = 0;

		return true;
	}

	void Shutdown() {
		delete[] s_QuadVertexBufferData;
		s_QuadVertexBufferData = nullptr;
		s_QuadVertexBufferCurrent = nullptr;

		s_TextureSlots.fill(InvalidTextureHandle);
		s_TextureSlotIndex = 0;

		s_QuadIndexCount = 0;
	}

	void BeginScene(Camera2D& camera) {
		camera.UpdateViewProj();
	}

	void EndScene() {
		Flush();
	}

	void Flush() {
		// Flush rendering commands here
	}
}
