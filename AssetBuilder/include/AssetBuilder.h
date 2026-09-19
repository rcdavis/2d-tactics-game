#pragma once

#include <filesystem>

class AssetBuilder {
public:
	AssetBuilder() = default;

	void BuildAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, const std::filesystem::path& generatedDir);
};
