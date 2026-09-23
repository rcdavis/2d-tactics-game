#pragma once

#include <cstdint>

using TileSetHandle = uint8_t;
using TileMapHandle = uint8_t;

constexpr TileSetHandle InvalidTileSetHandle = static_cast<TileSetHandle>(-1);
constexpr TileMapHandle InvalidTileMapHandle = static_cast<TileMapHandle>(-1);

