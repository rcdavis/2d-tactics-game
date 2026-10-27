#include "AssetBuilder.h"

#include <cstdint>
#include <iostream>
#include <fstream>
#include <cstring>
#include <regex>
#include <vector>

#include "ItemData.h"
#include "TileMapData.h"
#include "pugixml.hpp"
#include "nlohmann/json.hpp"

void AssetBuilder::BuildAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir) {
	std::filesystem::create_directories(outputDir);
	std::filesystem::create_directories(generatedDir);

	BuildTextures(inputDir, generatedDir);
	BuildShaders(inputDir, generatedDir);
	BuildTileMaps(inputDir, outputDir, generatedDir);
	BuildStrings(inputDir, outputDir, generatedDir);
	BuildItems(inputDir, outputDir, generatedDir);
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

void AssetBuilder::BuildShaders(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir) {
	for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir / "shaders")) {
		if (entry.is_regular_file()) {
			const auto& path = entry.path();
			if (path.extension() == ".vs") {
				const auto fragmentPath = path.parent_path() / (path.stem().string() + ".fs");
				if (std::filesystem::exists(fragmentPath)) {
					mShaders.push_back({path, fragmentPath});
					std::cout << "Found shader: " << path << " and " << fragmentPath << std::endl;
				}
			}
		}
	}

	CreateShaderIdHeader(inputDir, generatedDir);
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

void AssetBuilder::BuildStrings(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir) {
	for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir / "strings")) {
		if (entry.is_regular_file()) {
			const auto& path = entry.path();
			if (path.extension() == ".json") {
				mStringFiles.push_back(path);
				std::cout << "Found string file: " << path << std::endl;
			}
		}
	}

	const auto stringsOutputDir = outputDir / "strings";
	std::filesystem::create_directories(stringsOutputDir);

	for (const auto& stringFile : mStringFiles)
		ConvertStringFileToBinary(stringFile, stringsOutputDir);

	if (!mStringFiles.empty()) {
		std::vector<ParsedStringEntry> stringData;
		ParseStringData(mStringFiles[0], stringData);
		CreateStringIdHeader(generatedDir, stringData);
	}
}

void AssetBuilder::BuildItems(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir) {
	for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir / "items")) {
		if (entry.is_regular_file()) {
			const auto& path = entry.path();
			if (path.extension() == ".json") {
				mItemFiles.push_back(path);
				std::cout << "Found item file: " << path << std::endl;
			}
		}
	}

	const auto itemsOutputDir = outputDir / "items";
	std::filesystem::create_directories(itemsOutputDir);

	for (const auto& itemFile : mItemFiles)
		ConvertItemFileToBinary(itemFile, itemsOutputDir);

	if (!mItemFiles.empty()) {
		ParsedItemData itemData;
		ParseItemData(mItemFiles[0], itemData);
		CreateItemIdHeader(generatedDir, itemData);
	}
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

void AssetBuilder::ConvertStringFileToBinary(const std::filesystem::path& stringFilePath, const std::filesystem::path& outputDir) {
	std::vector<ParsedStringEntry> stringData;
	if (!ParseStringData(stringFilePath, stringData)) {
		std::cerr << "Failed to get string data: " << stringFilePath << std::endl;
		return;
	}

	const auto outputBinaryPath = outputDir / (stringFilePath.stem().string() + ".strbin");
	std::ofstream file(outputBinaryPath, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to create output binary file: " << outputBinaryPath << std::endl;
		return;
	}

	std::vector<StringEntry> stringEntries;
	std::vector<StringForm> stringForms;
	std::vector<char> stringPool;

	for (const auto& parsedEntry : stringData) {
		const StringEntry entry {
			.firstForm = static_cast<uint32_t>(stringForms.size()),
			.formCount = static_cast<uint32_t>(parsedEntry.forms.size())
		};

		for (const auto& parsedForm : parsedEntry.forms) {
			const StringForm form {
				.category = parsedForm.category,
				.offset = static_cast<uint32_t>(stringPool.size())
			};

			stringPool.insert(stringPool.end(), parsedForm.text.cbegin(), parsedForm.text.cend());
			stringPool.push_back('\0');

			stringForms.push_back(form);
		}

		stringEntries.push_back(entry);
	}

	const StringBinaryHeader header {
		.magic = {'S', 'T', 'R', 'B'},
		.version = 1,
		.entryCount = static_cast<uint32_t>(stringData.size()),
		.formCount = static_cast<uint32_t>(stringForms.size()),
		.stringPoolSize = static_cast<uint32_t>(stringPool.size())
	};

	file.write((const char*)&header, sizeof(StringBinaryHeader));
	file.write((const char*)stringEntries.data(), stringEntries.size() * sizeof(StringEntry));
	file.write((const char*)stringForms.data(), stringForms.size() * sizeof(StringForm));
	file.write((const char*)stringPool.data(), stringPool.size() * sizeof(char));

	std::cout << "Converted string file to binary successfully: " << stringFilePath << " -> " << outputBinaryPath << std::endl;
}

void AssetBuilder::ConvertItemFileToBinary(const std::filesystem::path& itemFilePath, const std::filesystem::path& outputDir) {
	ParsedItemData itemData;
	if (!ParseItemData(itemFilePath, itemData)) {
		std::cerr << "Failed to get item data: " << itemFilePath << std::endl;
		return;
	}

	const auto outputBinaryPath = outputDir / (itemFilePath.stem().string() + ".itmbin");
	std::ofstream file(outputBinaryPath, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to create output binary file: " << outputBinaryPath << std::endl;
		return;
	}

	if (mStringFiles.empty()) {
		std::cerr << "No string files available to generate item names." << std::endl;
		return;
	}

	std::vector<ParsedStringEntry> stringData;
	ParseStringData(mStringFiles[0], stringData);

	std::vector<WeaponData> weapons;
	weapons.reserve(itemData.weapons.size());
	for (const auto& weapon : itemData.weapons) {
		uint8_t nameId = 0;
		for (size_t i = 0; i < stringData.size(); ++i) {
			if (stringData[i].key == weapon.name) {
				nameId = i;
				break;
			}
		}
		weapons.push_back(WeaponData {
			.id = weapon.id,
			.nameId = nameId,
			.might = weapon.might,
			.hit = weapon.hit,
			.crit = weapon.crit,
			.minRange = weapon.minRange,
			.maxRange = weapon.maxRange,
			.maxDurability = weapon.maxDurability,
		});
	}

	std::vector<ConsumableData> consumables;
	consumables.reserve(itemData.consumables.size());
	for (const auto& consumable : itemData.consumables) {
		uint8_t nameId = 0;
		for (size_t i = 0; i < stringData.size(); ++i) {
			if (stringData[i].key == consumable.name) {
				nameId = i;
				break;
			}
		}
		consumables.push_back(ConsumableData {
			.id = consumable.id,
			.nameId = nameId,
			.effect = consumable.effect,
			.amount = consumable.amount,
			.maxUses = consumable.maxUses,
		});
	}

	const ItemBinHeader header {
		.magic = {'I', 'T', 'M', 'B'},
		.weaponCount = static_cast<uint16_t>(weapons.size()),
		.consumableCount = static_cast<uint16_t>(consumables.size())
	};
	file.write((const char*)&header, sizeof(ItemBinHeader));
	file.write((const char*)weapons.data(), weapons.size() * sizeof(WeaponData));
	file.write((const char*)consumables.data(), consumables.size() * sizeof(ConsumableData));

	std::cout << "Converted item file to binary successfully: " << itemFilePath << " -> " << outputBinaryPath << std::endl;
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

void AssetBuilder::CreateShaderIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir) {
	const auto headerPath = generatedDir / "ShaderIds.h";
	std::ofstream file(headerPath);
	if (!file) {
		std::cerr << "Failed to create shader ID header file: " << headerPath << std::endl;
		return;
	}

	const auto resParentDir = inputDir / "..";

	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "// This file is auto-generated. Do not modify directly.\n";
	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "#pragma once\n\n";
	file << "#include <cstdint>\n";
	file << "#include <array>\n\n";

	file << "namespace Res::Shaders::Vertex {\n";

	file << "\tenum class Id : uint8_t {\n";
	for (const auto& shader : mShaders) {
		file << "\t\t" << shader.first.stem().string() << ",\n";
	}
	file << "\t\tCount\n";
	file << "\t};\n\n";

	file << "\tinline constexpr const char* ToString(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& shader : mShaders) {
		file << "\t\t\tcase Id::" << shader.first.stem().string() << ": return \"" << shader.first.stem().string() << "\";\n";
	}
	file << "\t\t\tdefault: return \"Unknown\";\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr const char* GetPath(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& shader : mShaders) {
		const auto relativePath = std::filesystem::relative(shader.first, resParentDir);
		file << "\t\t\tcase Id::" << shader.first.stem().string() << ": return \"" << relativePath.generic_string() << "\";\n";
	}
	file << "\t\t\tdefault: return nullptr;\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr std::array<const char*, " << std::size(mShaders) << "> Paths = {\n";
	for (const auto& shader : mShaders) {
		const auto relativePath = std::filesystem::relative(shader.first, resParentDir);
		file << "\t\t\"" << relativePath.generic_string() << "\",\n";
	}
	file << "\t};\n";

	file << "} // namespace Res::Shaders::Vertex\n\n";

	file << "namespace Res::Shaders::Fragment {\n";

	file << "\tenum class Id : uint8_t {\n";
	for (const auto& shader : mShaders) {
		file << "\t\t" << shader.second.stem().string() << ",\n";
	}
	file << "\t\tCount\n";
	file << "\t};\n\n";

	file << "\tinline constexpr const char* ToString(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& shader : mShaders) {
		file << "\t\t\tcase Id::" << shader.second.stem().string() << ": return \"" << shader.second.stem().string() << "\";\n";
	}
	file << "\t\t\tdefault: return \"Unknown\";\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr const char* GetPath(Id id) {\n";
	file << "\t\tswitch (id) {\n";
	for (const auto& shader : mShaders) {
		const auto relativePath = std::filesystem::relative(shader.second, resParentDir);
		file << "\t\t\tcase Id::" << shader.second.stem().string() << ": return \"" << relativePath.generic_string() << "\";\n";
	}
	file << "\t\t\tdefault: return nullptr;\n";
	file << "\t\t}\n";
	file << "\t}\n\n";

	file << "\tinline constexpr std::array<const char*, " << std::size(mShaders) << "> Paths = {\n";
	for (const auto& shader : mShaders) {
		const auto relativePath = std::filesystem::relative(shader.second, resParentDir);
		file << "\t\t\"" << relativePath.generic_string() << "\",\n";
	}
	file << "\t};\n";

	file << "} // namespace Res::Shaders::Fragment\n";

	std::cout << "Generated shader ID header at " << headerPath << std::endl;
}


void AssetBuilder::CreateTileIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir) {
	const auto headerPath = generatedDir / "TileIds.h";
	std::ofstream file(headerPath);
	if (!file) {
		std::cerr << "Failed to create tile ID header file: " << headerPath << std::endl;
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

void AssetBuilder::CreateStringIdHeader(
	const std::filesystem::path& generatedDir,
	const std::vector<ParsedStringEntry>& stringData
) {
	const auto headerPath = generatedDir / "StringIds.h";
	std::ofstream file(headerPath);
	if (!file) {
		std::cerr << "Failed to create string ID header at " << headerPath << std::endl;
		return;
	}

	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "// This file is auto-generated. Do not modify directly.\n";
	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "#pragma once\n\n";
	file << "#include <cstdint>\n\n";

	file << "namespace Res::Strings {\n";

	file << "\tenum class Id : uint8_t {\n";
	for (const auto& entry : stringData) {
		file << "\t\t" << entry.key << ",\n";
	}
	file << "\t\tCount\n";
	file << "\t};\n";

	file << "} // namespace Res::Strings\n";

	std::cout << "Generated string ID header at " << headerPath << std::endl;
}

void AssetBuilder::CreateItemIdHeader(
		const std::filesystem::path& generatedDir,
		const ParsedItemData& itemData)
{
	const auto headerPath = generatedDir / "ItemIds.h";
	std::ofstream file(headerPath);
	if (!file) {
		std::cerr << "Failed to create item ID header at " << headerPath << std::endl;
		return;
	}

	std::vector<std::pair<uint8_t, std::string>> ids;
	ids.reserve(itemData.weapons.size() + itemData.consumables.size());
	for (const auto& weapon : itemData.weapons) {
		ids.push_back({weapon.id, weapon.name});
	}
	for (const auto& consumable : itemData.consumables) {
		ids.push_back({consumable.id, consumable.name});
	}

	std::sort(ids.begin(), ids.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;
	});

	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "// This file is auto-generated. Do not modify directly.\n";
	file << "////////////////////////////////////////////////////////////////////////\n";
	file << "#pragma once\n\n";
	file << "#include <cstdint>\n\n";

	file << "namespace Res::Items {\n";

	file << "\tenum class Id : uint8_t {\n";
	for (const auto& entry : ids) {
		file << "\t\t" << entry.second << ",\n";
	}
	file << "\t\tCount\n";
	file << "\t};\n";

	file << "} // namespace Res::Items\n";

	std::cout << "Generated item ID header at " << headerPath << std::endl;
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

bool AssetBuilder::ParseStringData(const std::filesystem::path& stringFilePath, std::vector<ParsedStringEntry>& outStringData) {
	std::ifstream file(stringFilePath);
	if (!file) {
		std::cerr << "Failed to open string file " << stringFilePath << std::endl;
		return false;
	}

	const nlohmann::json root = nlohmann::json::parse(file, nullptr, false);
	file.close();
	if (root.is_discarded()) {
		std::cerr << "Failed to parse string file " << stringFilePath << std::endl;
		return false;
	}

	for (const auto& [key, value] : root.items()) {
		ParsedStringEntry parsedEntry;
		parsedEntry.key = key;

		if (value.is_string()) {
			const ParsedStringForm parsedForm {
				.text = value.get<std::string>(),
				.category = PluralCategory::One
			};
			parsedEntry.forms.push_back(parsedForm);
		} else if (value.is_object()) {
			for (const auto& [categoryKey, categoryValue] : value.items()) {
				const ParsedStringForm parsedForm {
					.text = categoryValue.get<std::string>(),
					.category = StringToPluralCategory(categoryKey.c_str())
				};
				parsedEntry.forms.push_back(parsedForm);
			}
		}

		outStringData.push_back(parsedEntry);
	}

	return true;
}

bool AssetBuilder::ParseItemData(const std::filesystem::path& itemFilePath, ParsedItemData& outItemData) {
	std::ifstream file(itemFilePath);
	if (!file) {
		std::cerr << "Failed to open item file " << itemFilePath << std::endl;
		return false;
	}

	const nlohmann::json root = nlohmann::json::parse(file, nullptr, false);
	file.close();
	if (root.is_discarded()) {
		std::cerr << "Failed to parse item file " << itemFilePath << std::endl;
		return false;
	}

	const auto weaponArray = root["weapons"];
	if (weaponArray.is_array()) {
		for (const auto& weaponJson : weaponArray) {
			ParsedWeaponData weaponData;
			weaponData.name = weaponJson["name"].get<std::string>();
			weaponData.id = weaponJson["id"].get<uint8_t>();
			weaponData.might = weaponJson["might"].get<uint8_t>();
			weaponData.hit = weaponJson["hit"].get<uint8_t>();
			weaponData.crit = weaponJson["crit"].get<uint8_t>();
			weaponData.minRange = weaponJson["minRange"].get<uint8_t>();
			weaponData.maxRange = weaponJson["maxRange"].get<uint8_t>();
			weaponData.maxDurability = weaponJson["maxDurability"].get<uint8_t>();
			outItemData.weapons.push_back(weaponData);
		}
	}

	const auto consumableArray = root["consumables"];
	if (consumableArray.is_array()) {
		for (const auto& consumableJson : consumableArray) {
			ParsedConsumableData consumableData;
			consumableData.id = consumableJson["id"].get<uint8_t>();
			consumableData.name = consumableJson["name"].get<std::string>();
			consumableData.effect = StringToEffect(consumableJson["effect"].get<std::string>().c_str());
			consumableData.amount = consumableJson["amount"].get<uint8_t>();
			consumableData.maxUses = consumableJson["maxUses"].get<uint8_t>();
			outItemData.consumables.push_back(consumableData);
		}
	}

	return true;
}
