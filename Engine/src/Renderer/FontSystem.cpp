#include "Renderer/FontSystem.h"

#include "Renderer/Font.h"
#include "nlohmann/json.hpp"
#include "Utils/Log.h"

#include <vector>
#include <fstream>

namespace FontSystem {
	static std::vector<Font> s_Fonts;

	bool Load(const char* const fontFile, TextureHandle fontImage) {
		std::ifstream file(fontFile);
		if (!file) {
			LOG_ERROR("Failed to load font file: {}", fontFile);
			return false;
		}

		auto root = nlohmann::json::parse(file);
		file.close();

		Font font;
		font.texture = fontImage;

		const auto& atlasJson = root["atlas"];
		const float atlasWidth = atlasJson["width"];
		const float atlasHeight = atlasJson["height"];
		const bool flipY = atlasJson["yOrigin"] == "bottom";
		font.distanceRange = atlasJson["distanceRange"];

		const auto& metricsJson = root["metrics"];
		font.lineHeight = metricsJson["lineHeight"];
		font.ascender = metricsJson["ascender"];
		font.descender = metricsJson["descender"];

		for (const auto& glyphJson : root["glyphs"]) {
			const auto& planeBounds = glyphJson["planeBounds"];
			const auto& atlasBounds = glyphJson["atlasBounds"];
			Glyph glyph;
			glyph.planeMin = { planeBounds["left"], planeBounds["bottom"] };
			glyph.planeMax = { planeBounds["right"], planeBounds["top"] };
			float u0 = (float)atlasBounds["left"] / atlasWidth;
			float v0 = (float)atlasBounds["bottom"] / atlasHeight;
			float u1 = (float)atlasBounds["right"] / atlasWidth;
			float v1 = (float)atlasBounds["top"] / atlasHeight;
			if (flipY) {
				v0 = 1.0f - v0;
				v1 = 1.0f - v1;
			}
			glyph.uvMin = { u0, v1 };
			glyph.uvMax = { u1, v0 };
			glyph.hasGeometry = true;

			font.glyphs[glyphJson["unicode"]] = glyph;
		}

		s_Fonts.push_back(font);

		return true;
	}

	Font* Get(FontHandle handle) {
		// Implementation here
		return s_Fonts.data() + handle;
	}
}
