#pragma once

#include <filesystem>
#include <vector>

class AssetBuilder {
public:
	AssetBuilder() = default;

	void BuildAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir);

private:
	void BuildTextures(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);
	void BuildTileMaps(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir);

	void ConvertTileSetToBinary(const std::filesystem::path& tileSetPath, const std::filesystem::path& outputDir);

	void CreateTextureIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);

private:
	std::vector<std::filesystem::path> mTextures;
	std::vector<std::filesystem::path> mTileSets;
	std::vector<std::filesystem::path> mTileMaps;
};
