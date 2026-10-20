#pragma once

#include <filesystem>
#include <vector>

#include "TileMapData.h"
#include "StringData.h"

class AssetBuilder {
public:
	AssetBuilder() = default;

	void BuildAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir);

private:
	void BuildTextures(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);
	void BuildShaders(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);
	void BuildTileMaps(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir);
	void BuildStrings(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir);

	void ConvertTileSetToBinary(const std::filesystem::path& tileSetPath, const std::filesystem::path& outputDir);
	void ConvertTileMapToBinary(const std::filesystem::path& tileMapPath, const std::filesystem::path& outputDir);
	void ConvertStringFileToBinary(const std::filesystem::path& stringFilePath, const std::filesystem::path& outputDir);

	void CreateTextureIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);
	void CreateShaderIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);
	void CreateTileIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);
	void CreateStringIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);

	bool ParseTileSetData(const std::filesystem::path& tileSetPath, TileSetData& outTileSetData);
	bool ParseTileMapData(const std::filesystem::path& tileMapPath, TileMapData& outTileMapData);
	bool ParseStringData(const std::filesystem::path& stringFilePath, std::vector<ParsedStringEntry>& outStringData);

private:
	std::vector<std::filesystem::path> mTextures;
	std::vector<std::pair<std::filesystem::path, std::filesystem::path>> mShaders;
	std::vector<std::filesystem::path> mTileSets;
	std::vector<std::filesystem::path> mTileMaps;
	std::vector<std::filesystem::path> mStringFiles;
};
