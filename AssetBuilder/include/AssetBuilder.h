#pragma once

#include <filesystem>
#include <vector>

class AssetBuilder {
public:
	AssetBuilder() = default;

	void BuildAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir);

private:
	void BuildTextures(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);

	void CreateTextureIdHeader(const std::filesystem::path& inputDir, const std::filesystem::path& generatedDir);

private:
	std::vector<std::filesystem::path> mTextures;
};
