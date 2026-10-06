#pragma once

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

	void DrawTexturedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, TextureHandle textureHandle);

	void DrawTileMap(Camera2D& camera, TileMapHandle tileMapHandle);
}
