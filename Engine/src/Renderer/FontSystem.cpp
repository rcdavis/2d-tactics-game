#include "Renderer/FontSystem.h"

#include "Renderer/Font.h"
#include "nlohmann/json.hpp"
#include "Utils/Log.h"

#include <vector>
#include <fstream>

namespace FontSystem {
	static std::vector<Font> s_Fonts;

	FontHandle Load(const char* const fontFile, TextureHandle fontImage) {
		std::ifstream file(fontFile);
		if (!file) {
			LOG_ERROR("Failed to load font file: {}", fontFile);
			return InvalidFontHandle;
		}

		auto root = nlohmann::json::parse(file, nullptr, false);
		file.close();
		if (root.is_discarded()) {
			LOG_ERROR("Failed to parse font file: {}", fontFile);
			return InvalidFontHandle;
		}

		Font font;
		font.texture = fontImage;

		const auto& atlasJson = root["atlas"];
		const float atlasWidth = atlasJson["width"];
		const float atlasHeight = atlasJson["height"];
		// Textures are uploaded flipped (stbi_set_flip_vertically_on_load), so v = 0 is the bottom row
		const bool originTop = atlasJson["yOrigin"] == "top";
		font.distanceRange = atlasJson["distanceRange"];
		font.atlasSize = atlasJson["size"];

		const auto& metricsJson = root["metrics"];
		font.lineHeight = metricsJson["lineHeight"];
		font.ascender = metricsJson["ascender"];
		font.descender = metricsJson["descender"];

		for (const auto& glyphJson : root["glyphs"]) {
			Glyph glyph;
			glyph.advance = glyphJson["advance"];

			// Glyphs such as space have an advance but no geometry
			if (glyphJson.contains("planeBounds") && glyphJson.contains("atlasBounds")) {
				const auto& planeBounds = glyphJson["planeBounds"];
				const auto& atlasBounds = glyphJson["atlasBounds"];
				glyph.planeMin = { planeBounds["left"], planeBounds["bottom"] };
				glyph.planeMax = { planeBounds["right"], planeBounds["top"] };

				const float u0 = (float)atlasBounds["left"] / atlasWidth;
				const float u1 = (float)atlasBounds["right"] / atlasWidth;
				float vBottom = (float)atlasBounds["bottom"] / atlasHeight;
				float vTop = (float)atlasBounds["top"] / atlasHeight;
				if (originTop) {
					vBottom = 1.0f - vBottom;
					vTop = 1.0f - vTop;
				}
				glyph.uvMin = { u0, vBottom };
				glyph.uvMax = { u1, vTop };
				glyph.hasGeometry = true;
			}

			font.glyphs[glyphJson["unicode"]] = glyph;
		}

		s_Fonts.push_back(std::move(font));

		return static_cast<FontHandle>(s_Fonts.size() - 1);
	}

	Font* Get(FontHandle handle) {
		if (handle >= s_Fonts.size())
			return nullptr;
		return s_Fonts.data() + handle;
	}
}
