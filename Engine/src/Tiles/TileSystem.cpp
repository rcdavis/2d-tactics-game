#include "Tiles/TileSystem.h"

#include <vector>
#include <filesystem>

#include "Tiles/TileMap.h"

namespace TileSystem {
	static std::vector<TileMap> s_tileMaps;
	static std::vector<TileSet> s_tileSets;
	static std::vector<std::filesystem::path> s_tileMapPaths;

	bool Init() {
		// Initialization code here
		return true;
	}

	void Shutdown() {
		for (auto& tileMap : s_tileMaps)
			tileMap.Destroy();

		for (auto& tileSet : s_tileSets)
			tileSet.Destroy();

		s_tileMaps.clear();
		s_tileSets.clear();
		s_tileMapPaths.clear();
	}
} // namespace TileSystem
