#include "Renderer/Renderer2D.h"

#include <array>
#include <cstdint>

#include "Platform.h"

#include "Renderer/Camera2D.h"
#include "Renderer/TextureHandle.h"
#include "Renderer/IVertexArray.h"

#include "Tiles/TileSystem.h"
#include "Tiles/TileMap.h"

#include "Utils/Log.h"

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

	static IRenderDevice* s_RenderDevice = nullptr;

	static QuadVertex* s_QuadVertexBufferData = nullptr;
	static QuadVertex* s_QuadVertexBufferCurrent = nullptr;

	static IVertexArray* s_QuadVertexArray = nullptr;

	static uint32_t s_QuadIndexCount = 0;

	static std::array<TextureHandle, MaxTextureSlots> s_TextureSlots {};
	static uint32_t s_TextureSlotIndex = 0;

	bool Init(Platform& platform) {
		s_RenderDevice = platform.renderDevice;

		s_QuadVertexBufferData = new QuadVertex[MaxVertices];
		s_QuadVertexBufferCurrent = s_QuadVertexBufferData;

		s_QuadVertexArray = IVertexArray::Create(platform.graphicsApi);
		if (!s_QuadVertexArray->Init()) {
			LOG_ERROR("Failed to initialize quad vertex array");
			return false;
		}

		s_TextureSlots.fill(InvalidTextureHandle);
		s_TextureSlotIndex = 0;

		s_QuadIndexCount = 0;

		return true;
	}

	void Shutdown() {
		s_RenderDevice = nullptr;

		delete[] s_QuadVertexBufferData;
		s_QuadVertexBufferData = nullptr;
		s_QuadVertexBufferCurrent = nullptr;

		delete s_QuadVertexArray;
		s_QuadVertexArray = nullptr;

		s_TextureSlots.fill(InvalidTextureHandle);
		s_TextureSlotIndex = 0;

		s_QuadIndexCount = 0;
	}

	void BeginScene(Camera2D& camera) {
		camera.UpdateViewProj();

		s_QuadVertexArray->Bind();
	}

	void EndScene() {
		Flush();

		s_QuadVertexArray->Unbind();
	}

	void Flush() {
		// Flush rendering commands here
	}

	void DrawTileMap(Camera2D& camera, TileMapHandle tileMapHandle) {
		const TileMap* const tileMap = TileSystem::GetTileMap(tileMapHandle);
		const TileSet* const tileSet = TileSystem::GetTileSet(tileMap->tileSetHandle);

		int16_t startRow = (int16_t)camera.position.y / (int16_t)tileMap->tileHeight;
		if (startRow < 0)
			startRow = 0;
		int16_t endRow = (int16_t)(camera.position.y + camera.height) / (int16_t)tileMap->tileHeight + 1;
		if (endRow > tileMap->tileRowCount)
			endRow = tileMap->tileRowCount;
		int16_t startCol = (int16_t)camera.position.x / (int16_t)tileMap->tileWidth;
		if (startCol < 0)
			startCol = 0;
		int16_t endCol = (int16_t)(camera.position.x + camera.width) / (int16_t)tileMap->tileWidth + 1;
		if (endCol > tileMap->tileColumnCount)
			endCol = tileMap->tileColumnCount;

		for (uint8_t layerIndex = 0; layerIndex < tileMap->layerCount; ++layerIndex) {
			const TileLayer& layer = tileMap->layers[layerIndex];
			const float zIndex = 0.1f * layerIndex;

			for (int16_t row = startRow; row < endRow; ++row) {
				for (int16_t col = startCol; col < endCol; ++col) {
					const uint16_t tileId = layer.tiles[row * tileMap->tileColumnCount + col];
					if (tileId == 0)
						continue;

					// Calculate the position and texture coordinates for the tile
					const float x = col * tileMap->tileWidth;
					const float y = row * tileMap->tileHeight;
					const float w = tileMap->tileWidth;
					const float h = tileMap->tileHeight;

					const std::array<glm::vec3, 4> vertPositions = {
						glm::vec3 { x, y, zIndex },
						glm::vec3 { x + w, y, zIndex },
						glm::vec3 { x + w, y - h, zIndex },
						glm::vec3 { x, y - h, zIndex }
					};

					const auto texCoords = tileSet->GetTexCoords(tileId);

					// Add the quad vertices to the buffer
					for (size_t i = 0; i < 4; ++i) {
						s_QuadVertexBufferCurrent->position = vertPositions[i];
						s_QuadVertexBufferCurrent->color = {1.0f, 1.0f, 1.0f, 1.0f};
						s_QuadVertexBufferCurrent->texCoord = texCoords[i];
						s_QuadVertexBufferCurrent->texIndex = 0;
						++s_QuadVertexBufferCurrent;
					}

					s_QuadIndexCount += 6;
				}
			}
		}
	}
}
