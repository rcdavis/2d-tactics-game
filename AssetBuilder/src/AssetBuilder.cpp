#include "AssetBuilder.h"

#include <iostream>
#include <fstream>

#include "TileMapData.h"
#include "pugixml.hpp"

void AssetBuilder::BuildAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir) {
	std::filesystem::create_directories(outputDir);
	std::filesystem::create_directories(generatedDir);

	BuildTextures(inputDir, generatedDir);
	BuildTileMaps(inputDir, outputDir, generatedDir);
}

void AssetBuilder::BuildTextures(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir) {
	for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir / "textures")) {
		if (entry.is_regular_file()) {
			const auto& path = entry.path();
			if (path.extension() == ".png") {
				mTextures.push_back(path);
				std::cout << "Found texture: " << path << std::endl;
			}
		}
	}

	CreateTextureIdHeader(inputDir, generatedDir);
}

void AssetBuilder::BuildTileMaps(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir) {
	for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir / "tilemaps")) {
		if (entry.is_regular_file()) {
			const auto& path = entry.path();
			if (path.extension() == ".tsx") {
				mTileSets.push_back(path);
				std::cout << "Found tile set: " << path << std::endl;
			} else if (path.extension() == ".tmx") {
				mTileMaps.push_back(path);
				std::cout << "Found tile map: " << path << std::endl;
			}
		}
	}

	const auto tilemapsOutputDir = outputDir / "tilemaps";
	std::filesystem::create_directories(tilemapsOutputDir);

	for (const auto& tileSet : mTileSets)
		ConvertTileSetToBinary(tileSet, tilemapsOutputDir);
}

void AssetBuilder::ConvertTileSetToBinary(const std::filesystem::path& tileSetPath, const std::filesystem::path& outputDir) {
	TileSetData tileSetData;
	if (!ParseTileSetData(tileSetPath, tileSetData)) {
		std::cerr << "Failed to get tile set data: " << tileSetPath << std::endl;
		return;
	}

	const auto outputBinaryPath = outputDir / (tileSetPath.stem().string() + ".tsxbin");
	std::ofstream outputFile(outputBinaryPath, std::ios::binary);
	if (!outputFile) {
		std::cerr << "Failed to create output binary file: " << outputBinaryPath << std::endl;
		return;
	}

	uint16_t imageHandle = -1;
	for (size_t i = 0; i < std::size(mTextures); ++i) {
		if (mTextures[i].filename() == tileSetData.imagePath.filename()) {
			imageHandle = static_cast<uint16_t>(i);
			break;
		}
	}

	if (imageHandle == static_cast<uint16_t>(-1)) {
		std::cerr << "Failed to find image handle for tile set: " << tileSetPath << std::endl;
		return;
	}

	const TileSetBinaryHeader header {
		.imageHandle = imageHandle,
		.imageWidth = tileSetData.imageWidth,
		.imageHeight = tileSetData.imageHeight,
		.tileWidth = tileSetData.tileWidth,
		.tileHeight = tileSetData.tileHeight,
		.tileCount = tileSetData.tileCount,
		.columnCount = tileSetData.columnCount
	};

	outputFile.write((const char*)&header, sizeof(TileSetBinaryHeader));
	outputFile.write((const char*)std::data(tileSetData.terrains),
		std::size(tileSetData.terrains) * sizeof(TileTerrainData));

	std::cout << "Converted tile set to binary successfully: " << tileSetPath << " -> " << outputBinaryPath << std::endl;
}

void AssetBuilder::CreateTextureIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir) {
	const auto headerPath = generatedDir / "TextureIds.h";
	std::ofstream file(headerPath);
	if (!file) {
		std::cerr << "Failed to create texture ID header file: " << headerPath << std::endl;
		return;
	}

	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "// This file is auto-generated. Do not modify directly.\n";
	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "#pragma once\n\n";
	file << "#include <cstdint>\n";
	file << "#include <array>\n\n";

	file << "namespace Res::Textures {\n";

	file << "\tenum class Id : uint8_t {\n";
	for (const auto& texture : mTextures) {
		file << "\t\t" << texture.stem().string() << ",\n";
	}
	file << "\t\tCount\n";
	file << "\t};\n\n";

	file << "\tinline constexpr const char* ToString(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& path : mTextures) {
		file << "\t\t\tcase Id::" << path.stem().string() << ": return \"" << path.stem().string() << "\";\n";
	}
	file << "\t\t\tdefault: return \"Unknown\";\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	const auto resParentDir = inputDir / "..";
	file << "\tinline constexpr const char* GetPath(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& path : mTextures) {
		const auto relativePath = std::filesystem::relative(path, resParentDir);
		file << "\t\t\tcase Id::" << path.stem().string() << ": return \"" << relativePath.generic_string() << "\";\n";
	}
	file << "\t\t\tdefault: return nullptr;\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr std::array<const char*, " << std::size(mTextures) << "> Paths = {\n";
	for (const auto& path : mTextures) {
		const auto relativePath = std::filesystem::relative(path, resParentDir);
		file << "\t\t\"" << relativePath.generic_string() << "\",\n";
	}
	file << "\t};\n";

	file << "} // namespace Res::Textures\n";

	std::cout << "Generated texture ID header at " << headerPath << std::endl;
}

bool AssetBuilder::ParseTileSetData(const std::filesystem::path& tileSetPath, TileSetData& outTileSetData) {
	pugi::xml_document doc;
	if (const pugi::xml_parse_result result = doc.load_file(tileSetPath.c_str()); !result) {
		std::cerr << "Failed to parse tsx file " << tileSetPath << ": " << result.description() << std::endl;
		return false;
	}

	const pugi::xml_node tilesetNode = doc.child("tileset");
	if (!tilesetNode) {
		std::cerr << "Invalid tsx: missing tileset node in " << tileSetPath << std::endl;
		return false;
	}

	const pugi::xml_node imageNode = tilesetNode.child("image");
	if (!imageNode) {
		std::cerr << "Invalid tsx: missing image node in " << tileSetPath << std::endl;
		return false;
	}

	outTileSetData.imagePath = imageNode.attribute("source").as_string();
	outTileSetData.imageWidth = imageNode.attribute("width").as_uint();
	outTileSetData.imageHeight = imageNode.attribute("height").as_uint();

	outTileSetData.tileWidth = tilesetNode.attribute("tilewidth").as_uint();
	outTileSetData.tileHeight = tilesetNode.attribute("tileheight").as_uint();
	outTileSetData.tileCount = tilesetNode.attribute("tilecount").as_uint();
	outTileSetData.columnCount = tilesetNode.attribute("columns").as_uint();

	outTileSetData.terrains.resize(outTileSetData.tileCount);
	for (pugi::xml_node tileNode = tilesetNode.child("tile"); tileNode; tileNode = tileNode.next_sibling("tile")) {
		const uint32_t tileId = tileNode.attribute("id").as_uint();
		const pugi::xml_node propertiesNode = tileNode.child("properties");
		if (!propertiesNode) {
			std::cerr << "Tile " << tileId << " is missing properties node in " << tileSetPath << std::endl;
			continue;
		}

		for (pugi::xml_node propertyNode = propertiesNode.child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property")) {
			const std::string propertyName = propertyNode.attribute("name").as_string();
			const std::string propertyType = propertyNode.attribute("type").as_string();

			if (propertyName == "movementCost" && propertyType == "int") {
				const uint8_t movementCost = (uint8_t)propertyNode.attribute("value").as_uint(1);
				outTileSetData.terrains[tileId].movementCost = movementCost;
			}
		}
	}

	return true;
}
