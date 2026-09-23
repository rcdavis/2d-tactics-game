#include "AssetBuilder.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <regex>

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

	for (const auto& tileMap : mTileMaps)
		ConvertTileMapToBinary(tileMap, tilemapsOutputDir);

	CreateTileIdHeader(inputDir, generatedDir);
}

void AssetBuilder::ConvertTileSetToBinary(const std::filesystem::path& tileSetPath, const std::filesystem::path& outputDir) {
	TileSetData tileSetData;
	if (!ParseTileSetData(tileSetPath, tileSetData)) {
		std::cerr << "Failed to get tile set data: " << tileSetPath << std::endl;
		return;
	}

	const auto outputBinaryPath = outputDir / (tileSetPath.stem().string() + ".tsxbin");
	std::ofstream file(outputBinaryPath, std::ios::binary);
	if (!file) {
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

	file.write((const char*)&header, sizeof(TileSetBinaryHeader));
	file.write((const char*)std::data(tileSetData.terrains),
		std::size(tileSetData.terrains) * sizeof(TileTerrainData));

	std::cout << "Converted tile set to binary successfully: " << tileSetPath << " -> " << outputBinaryPath << std::endl;
}

void AssetBuilder::ConvertTileMapToBinary(const std::filesystem::path& tileMapPath, const std::filesystem::path& outputDir) {
	TileMapData tileMapData;
	if (!ParseTileMapData(tileMapPath, tileMapData)) {
		std::cerr << "Failed to get tile map data: " << tileMapPath << std::endl;
		return;
	}

	const auto outputBinaryPath = outputDir / (tileMapPath.stem().string() + ".tmxbin");
	std::ofstream file(outputBinaryPath, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to create output binary file: " << outputBinaryPath << std::endl;
		return;
	}

	uint16_t tileSetHandle = -1;
	for (size_t i = 0; i < std::size(mTileSets); ++i) {
		if (mTileSets[i].filename() == tileMapData.tileSetPath.filename()) {
			tileSetHandle = static_cast<uint16_t>(i);
			break;
		}
	}

	if (tileSetHandle == static_cast<uint16_t>(-1)) {
		std::cerr << "Failed to find tile set handle for tile set: " << tileMapData.tileSetPath << std::endl;
		return;
	}

	const TileMapBinaryHeader header {
		.tileSetHandle = tileSetHandle,
		.tileRowCount = tileMapData.tileRowCount,
		.tileColumnCount = tileMapData.tileColumnCount,
		.tileWidth = tileMapData.tileWidth,
		.tileHeight = tileMapData.tileHeight,
		.layerCount = static_cast<uint16_t>(tileMapData.layers.size())
	};

	file.write((const char*)&header, sizeof(TileMapBinaryHeader));

	for (const auto& layer : tileMapData.layers) {
		file.write((const char*)std::data(layer.tileIds),
			std::size(layer.tileIds) * sizeof(uint16_t));
	}

	std::cout << "Converted tile map to binary successfully: " << tileMapPath << " -> " << outputBinaryPath << std::endl;
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

void AssetBuilder::CreateTileIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir) {
	const auto headerPath = generatedDir / "TileIds.h";
	std::ofstream file(headerPath);
	if (!file) {
		std::cerr << "Failed to create tile map ID header file: " << headerPath << std::endl;
		return;
	}

	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "// This file is auto-generated. Do not modify directly.\n";
	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "#pragma once\n\n";
	file << "#include <cstdint>\n";
	file << "#include <array>\n\n";

	file << "namespace Res::Tiles::Sets {\n";

	file << "\tenum class Id : uint8_t {\n";
	for (const auto& tileSet : mTileSets) {
		file << "\t\t" << tileSet.stem().string() << ",\n";
	}
	file << "\t\tCount\n";
	file << "\t};\n\n";

	file << "\tinline constexpr const char* ToString(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& tileSet : mTileSets) {
		file << "\t\t\tcase Id::" << tileSet.stem().string() << ": return \"" << tileSet.stem().string() << "\";\n";
	}
	file << "\t\t\tdefault: return \"Unknown\";\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	const auto resParentDir = inputDir / "..";
	file << "\tinline constexpr const char* GetPath(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& tileSet : mTileSets) {
		const auto relativePath = std::filesystem::relative(tileSet, resParentDir).replace_extension("tsxbin");
		file << "\t\t\tcase Id::" << tileSet.stem().string() << ": return \"" << relativePath.generic_string() << "\";\n";
	}
	file << "\t\t\tdefault: return nullptr;\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr std::array<const char*, " << std::size(mTileSets) << "> Paths = {\n";
	for (const auto& tileSet : mTileSets) {
		const auto relativePath = std::filesystem::relative(tileSet, resParentDir).replace_extension("tsxbin");
		file << "\t\t\"" << relativePath.generic_string() << "\",\n";
	}
	file << "\t};\n";

	file << "} // namespace Res::Tiles::Sets\n\n";

	file << "namespace Res::Tiles::Maps {\n";

	file << "\tenum class Id : uint8_t {\n";
	for (const auto& tileMap : mTileMaps) {
		file << "\t\t" << tileMap.stem().string() << ",\n";
	}
	file << "\t\tCount\n";
	file << "\t};\n\n";

	file << "\tinline constexpr const char* ToString(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& tileMap : mTileMaps) {
		file << "\t\t\tcase Id::" << tileMap.stem().string() << ": return \"" << tileMap.stem().string() << "\";\n";
	}
	file << "\t\t\tdefault: return \"Unknown\";\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr const char* GetPath(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& tileMap : mTileMaps) {
		const auto relativePath = std::filesystem::relative(tileMap, resParentDir).replace_extension("tmxbin");
		file << "\t\t\tcase Id::" << tileMap.stem().string() << ": return \"" << relativePath.generic_string() << "\";\n";
	}
	file << "\t\t\tdefault: return nullptr;\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr std::array<const char*, " << std::size(mTileMaps) << "> Paths = {\n";
	for (const auto& tileMap : mTileMaps) {
		const auto relativePath = std::filesystem::relative(tileMap, resParentDir).replace_extension("tmxbin");
		file << "\t\t\"" << relativePath.generic_string() << "\",\n";
	}
	file << "\t};\n";

	file << "} // namespace Res::Tiles::Maps\n";

	std::cout << "Generated tile ID header at " << headerPath << std::endl;
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

bool AssetBuilder::ParseTileMapData(const std::filesystem::path& tileMapPath, TileMapData& outTileMapData) {
	pugi::xml_document doc;
	if (const pugi::xml_parse_result result = doc.load_file(tileMapPath.c_str()); !result) {
		std::cerr << "Failed to parse tmx file " << tileMapPath << ": " << result.description() << std::endl;
		return false;
	}

	const pugi::xml_node mapNode = doc.child("map");
	if (!mapNode) {
		std::cerr << "Invalid tmx: missing map node in " << tileMapPath << std::endl;
		return false;
	}

	const pugi::xml_node tilesetNode = mapNode.child("tileset");
	if (!tilesetNode) {
		std::cerr << "Invalid tmx: missing tileset node in " << tileMapPath << std::endl;
		return false;
	}

	outTileMapData.tileSetPath = tilesetNode.attribute("source").as_string();
	outTileMapData.tileRowCount = mapNode.attribute("height").as_uint();
	outTileMapData.tileColumnCount = mapNode.attribute("width").as_uint();
	outTileMapData.tileWidth = mapNode.attribute("tilewidth").as_uint();
	outTileMapData.tileHeight = mapNode.attribute("tileheight").as_uint();

	outTileMapData.layers.clear();
	for (pugi::xml_node layerNode = mapNode.child("layer"); layerNode; layerNode = layerNode.next_sibling("layer")) {
		const std::string layerName = layerNode.attribute("name").as_string();

		const pugi::xml_node dataNode = layerNode.child("data");
		if (!dataNode) {
			std::cerr << "Layer \"" << layerName << "\" is missing data node in " << tileMapPath << std::endl;
			continue;
		}

		if (strcmp(dataNode.attribute("encoding").as_string(), "csv") != 0) {
			std::cerr << "Unsupported data encoding in layer \"" << layerName << "\" in " << tileMapPath << std::endl;
			continue;
		}

		const std::string csvText = dataNode.text().as_string();

		TileLayerData layerData;
		layerData.tileIds.clear();

		std::vector<std::string> csvValues;
		const std::regex csvRegex("[, \t\r\n]+");
		std::copy(
			std::sregex_token_iterator(std::cbegin(csvText), std::cend(csvText), csvRegex, -1),
			std::sregex_token_iterator(),
			std::back_inserter(csvValues)
		);

		for (const std::string& value : csvValues) {
			if (value.empty())
				continue;

			const uint16_t tileId = static_cast<uint16_t>(std::stoul(value));
			layerData.tileIds.push_back(tileId);
		}

		outTileMapData.layers.push_back(layerData);
	}

	return true;
}
