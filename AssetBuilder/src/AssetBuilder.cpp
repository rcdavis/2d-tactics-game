#include "AssetBuilder.h"

#include <iostream>
#include <fstream>

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
	const auto outputBinaryPath = outputDir / (tileSetPath.stem().string() + ".tsxbin");
	std::ofstream outputFile(outputBinaryPath, std::ios::binary);
	if (!outputFile) {
		std::cerr << "Failed to create output binary file: " << outputBinaryPath << std::endl;
		return;
	}

	std::cout << "Converting tile set to binary: " << tileSetPath << " -> " << outputBinaryPath << std::endl;
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
