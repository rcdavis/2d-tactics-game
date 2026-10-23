#pragma once

#include <string_view>

#include "Renderer/FontHandle.h"
#include "Renderer/ShaderHandle.h"
#include "Renderer/TextureHandle.h"
#include "Tiles/TileHandle.h"
#include "Renderer/ClearFlags.h"

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

struct Camera2D;
class IRenderDevice;

namespace Renderer2D {
	bool Init(IRenderDevice* renderDevice, ShaderHandle quadShaderHandle);

	void Shutdown();

	void Clear(ClearFlags flags);

	void BeginScene(Camera2D& camera);
	void EndScene();

	void Flush();

	void Present();

	void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

	void DrawTexturedQuad(
		const glm::vec3& position,
		const glm::vec2& size,
		const glm::vec4& color,
		TextureHandle textureHandle);

	// position is the top-left of the first line; size is the font size in world units (em height).
	// Requires a y-down camera; '\n' starts a new line.
	void DrawText(
		FontHandle fontHandle,
		std::string_view utf8Text,
		const glm::vec3& position,
		float size,
		const glm::vec4& color);

	void DrawTile(
		const glm::vec3& position,
		const glm::vec2& size,
		uint16_t tileId,
		const glm::vec4& color,
		TileSetHandle tileSetHandle);

	void DrawTileMap(Camera2D& camera, TileMapHandle tileMapHandle);
}
