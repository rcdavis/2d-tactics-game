#include "Tiles/TileSystem.h"

#include <vector>
#include <filesystem>
#include <cassert>

#include "Tiles/TileMap.h"

namespace TileSystem {
	static std::vector<TileMap> s_tileMaps;
	static std::vector<TileSet> s_tileSets;
	static std::vector<std::filesystem::path> s_tileMapPaths;
	static std::vector<std::filesystem::path> s_tileSetPaths;

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
		s_tileSetPaths.clear();
	}

	TileSetHandle LoadTileSet(const char* const path) {
		for (size_t i = 0; i < s_tileSetPaths.size(); ++i) {
			if (s_tileSetPaths[i] == path) {
				return static_cast<TileSetHandle>(i);
			}
		}

		TileSet tileSet;
		if (!tileSet.Init(path)) {
			return InvalidTileSetHandle;
		}
		s_tileSets.push_back(tileSet);
		s_tileSetPaths.push_back(path);
		return static_cast<TileSetHandle>(s_tileSets.size() - 1);
	}

	TileMapHandle LoadTileMap(const char* const path) {
		for (size_t i = 0; i < s_tileMapPaths.size(); ++i) {
			if (s_tileMapPaths[i] == path) {
				return static_cast<TileMapHandle>(i);
			}
		}

		TileMap tileMap;
		if (!tileMap.Init(path)) {
			return InvalidTileMapHandle;
		}
		s_tileMaps.push_back(tileMap);
		s_tileMapPaths.push_back(path);
		return static_cast<TileMapHandle>(s_tileMaps.size() - 1);
	}

	const TileSet* GetTileSet(TileSetHandle handle) {
		assert(static_cast<size_t>(handle) < s_tileSets.size() && "Invalid TileSetHandle");
		return &s_tileSets[handle];
	}

	const TileMap* GetTileMap(TileMapHandle handle) {
		assert(static_cast<size_t>(handle) < s_tileMaps.size() && "Invalid TileMapHandle");
		return &s_tileMaps[handle];
	}
} // namespace TileSystem
