#pragma once

#include <cstdint>

using ItemHandle = std::uint16_t;

constexpr ItemHandle InvalidItemHandle = static_cast<ItemHandle>(-1);
