#include "Renderer/Renderer2D.h"

#include <array>
#include <vector>
#include <cstdint>

#include "Renderer/Camera2D.h"
#include "Renderer/TextureHandle.h"
#include "Renderer/IRenderDevice.h"
#include "Renderer/IVertexArray.h"
#include "Renderer/IShader.h"
#include "Renderer/ITexture.h"
#include "Renderer/ShaderSystem.h"
#include "Renderer/Buffer.h"

#include "Renderer/TextureSystem.h"
#include "Tiles/TileSystem.h"
#include "Tiles/TileMap.h"

#include "Utils/Log.h"

#include "glm/ext/matrix_transform.hpp"

namespace Renderer2D {
	struct QuadVertex {
		glm::vec3 position {0.0f, 0.0f, 0.0f};
		glm::vec4 color {1.0f, 1.0f, 1.0f, 1.0f};
		glm::vec2 texCoord {0.0f, 0.0f};
		int32_t texIndex = 0;
	};

	static constexpr uint32_t MaxQuads = 3'000;
	static constexpr uint32_t MaxVertices = MaxQuads * 4;
	static constexpr uint32_t MaxIndices = MaxQuads * 6;
	static constexpr uint32_t MaxTextureSlots = 32;

	static IRenderDevice* s_RenderDevice = nullptr;

	static QuadVertex* s_QuadVertexBufferData = nullptr;
	static QuadVertex* s_QuadVertexBufferCurrent = nullptr;

	static IVertexArray* s_QuadVertexArray = nullptr;
	static IVertexBuffer* s_QuadVertexBuffer = nullptr;
	static IIndexBuffer* s_QuadIndexBuffer = nullptr;

	static ITexture* s_WhiteTexture = nullptr;

	static IShader* s_QuadShader = nullptr;

	static uint32_t s_QuadIndexCount = 0;

	static std::array<TextureHandle, MaxTextureSlots> s_TextureSlots {};
	static uint32_t s_TextureSlotIndex = 0;

	static constexpr std::array<glm::vec4, 4> quadVertexPositions = {
		glm::vec4 { 0.0f, 0.0f, 0.0f, 1.0f },
		glm::vec4 { 1.0f, 0.0f, 0.0f, 1.0f },
		glm::vec4 { 1.0f, 1.0f, 0.0f, 1.0f },
		glm::vec4 { 0.0f, 1.0f, 0.0f, 1.0f }
	};

	bool Init(IRenderDevice* renderDevice, ShaderHandle quadShaderHandle) {
		s_QuadShader = ShaderSystem::GetShader(quadShaderHandle);
		if (!s_QuadShader) {
			LOG_ERROR("Failed to get quad shader");
			return false;
		}

		s_RenderDevice = renderDevice;

		s_QuadVertexBufferData = new QuadVertex[MaxVertices];
		s_QuadVertexBufferCurrent = s_QuadVertexBufferData;

		s_QuadVertexArray = s_RenderDevice->CreateVertexArray();
		if (!s_QuadVertexArray->Init()) {
			LOG_ERROR("Failed to initialize quad vertex array");
			return false;
		}

		s_QuadVertexBuffer = s_RenderDevice->CreateVertexBuffer();
		if (!s_QuadVertexBuffer->Init(BufferUsage::Dynamic, BufferLayout {
			{ BufferElementType::Float3, }, // Position
			{ BufferElementType::Float4, }, // Color
			{ BufferElementType::Float2, }, // TexCoord
			{ BufferElementType::Int, } // TexIndex
		}, s_QuadVertexBufferData, MaxVertices * sizeof(QuadVertex))
		) {
			LOG_ERROR("Failed to initialize quad vertex buffer");
			return false;
		}
		s_QuadVertexArray->SetVertexBuffer(s_QuadVertexBuffer);

		std::vector<uint16_t> indices(MaxIndices);
		for (uint16_t i = 0, offset = 0; i < MaxIndices; i += 6, offset += 4) {
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;
		}

		s_QuadIndexBuffer = s_RenderDevice->CreateIndexBuffer();
		if (!s_QuadIndexBuffer->Init(BufferUsage::Static, std::data(indices), std::size(indices))) {
			LOG_ERROR("Failed to initialize quad index buffer");
			return false;
		}
		s_QuadVertexArray->SetIndexBuffer(s_QuadIndexBuffer);

		s_TextureSlots.fill(InvalidTextureHandle);
		s_TextureSlotIndex = 0;

		constexpr uint32_t whiteTextureData = 0xFFFFFFFF; // RGBA8 white pixel
		s_WhiteTexture = s_RenderDevice->CreateTexture();
		if (!s_WhiteTexture->Init(&whiteTextureData, 1, 1)) {
			LOG_ERROR("Failed to initialize white texture");
			return false;
		}

		s_QuadIndexCount = 0;

		return true;
	}

	void Shutdown() {
		delete s_WhiteTexture;
		s_WhiteTexture = nullptr;

		s_QuadShader = nullptr;
		s_RenderDevice = nullptr;

		delete[] s_QuadVertexBufferData;
		s_QuadVertexBufferData = nullptr;
		s_QuadVertexBufferCurrent = nullptr;

		delete s_QuadIndexBuffer;
		s_QuadIndexBuffer = nullptr;

		delete s_QuadVertexBuffer;
		s_QuadVertexBuffer = nullptr;

		delete s_QuadVertexArray;
		s_QuadVertexArray = nullptr;

		s_TextureSlots.fill(InvalidTextureHandle);
		s_TextureSlotIndex = 0;

		s_QuadIndexCount = 0;
	}

	void Clear(ClearFlags flags) {
		s_RenderDevice->Clear(flags);
	}

	void BeginScene(Camera2D& camera) {
		camera.UpdateViewProj();

		s_QuadVertexArray->Bind();
		s_QuadShader->Bind();
		s_QuadShader->SetMat4("u_ViewProjection", camera.viewProj);

		s_TextureSlotIndex = 0;
	}

	void EndScene() {
		Flush();

		s_QuadVertexArray->Unbind();
	}

	void Flush() {
		if (s_QuadIndexCount == 0)
			return;

		const uint32_t vertCount = s_QuadVertexBufferCurrent - s_QuadVertexBufferData;
		s_QuadVertexBuffer->SetData(s_QuadVertexBufferData, vertCount * sizeof(QuadVertex));

		s_WhiteTexture->Bind(0);
		for (uint32_t i = 0; i < s_TextureSlotIndex; ++i) {
			ITexture* const texture = TextureSystem::GetTexture(s_TextureSlots[i]);
			texture->Bind(i + 1);
		}

		s_RenderDevice->DrawIndexed(s_QuadVertexArray, s_QuadIndexCount);
		s_QuadVertexBufferCurrent = s_QuadVertexBufferData;
		s_QuadIndexCount = 0;
	}

	void Present() {
		s_RenderDevice->Present();
	}

	void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
		if (s_QuadIndexCount >= MaxIndices)
			Flush();

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < 4; ++i) {
			s_QuadVertexBufferCurrent->position = transform * quadVertexPositions[i];
			s_QuadVertexBufferCurrent->color = color;
			s_QuadVertexBufferCurrent->texCoord = { 0.0f, 0.0f };
			s_QuadVertexBufferCurrent->texIndex = 0;
			++s_QuadVertexBufferCurrent;
		}

		s_QuadIndexCount += 6;
	}

	void DrawTileMap(Camera2D& camera, TileMapHandle tileMapHandle) {
		const TileMap* const tileMap = TileSystem::GetTileMap(tileMapHandle);
		const TileSet* const tileSet = TileSystem::GetTileSet(tileMap->tileSetHandle);

		// Reuse the slot if this texture is already bound; slot 0 is the white texture
		int32_t texIndex = 0;
		for (uint32_t i = 0; i < s_TextureSlotIndex; ++i) {
			if (s_TextureSlots[i] == tileSet->textureHandle) {
				texIndex = (int32_t)i + 1;
				break;
			}
		}
		if (texIndex == 0) {
			if (s_TextureSlotIndex >= MaxTextureSlots - 1)
				Flush();
			s_TextureSlots[s_TextureSlotIndex] = tileSet->textureHandle;
			++s_TextureSlotIndex;
			texIndex = (int32_t)s_TextureSlotIndex;
		}

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
			const float zIndex = 0.25f * layerIndex;

			for (int16_t row = startRow; row < endRow; ++row) {
				for (int16_t col = startCol; col < endCol; ++col) {
					const uint16_t tileId = layer.tiles[row * tileMap->tileColumnCount + col];
					if (tileId == 0)
						continue;

					if (s_QuadIndexCount >= MaxIndices) {
						Flush();
						s_TextureSlots[0] = tileSet->textureHandle;
						s_TextureSlotIndex = 1;
						texIndex = 1;
					}

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
						s_QuadVertexBufferCurrent->texIndex = texIndex;
						++s_QuadVertexBufferCurrent;
					}

					s_QuadIndexCount += 6;
				}
			}
		}
	}
}
