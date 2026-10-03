#pragma once

#include <type_traits>
#include <cstdint>

enum class ClearFlags : uint8_t {
	None = 0,
	Color = 1 << 0,
	Depth = 1 << 1,
	Stencil = 1 << 2,
};

constexpr ClearFlags operator|(ClearFlags a, ClearFlags b) {
	using T = std::underlying_type_t<ClearFlags>;
	return static_cast<ClearFlags>(static_cast<T>(a) | static_cast<T>(b));
}

constexpr ClearFlags operator&(ClearFlags a, ClearFlags b) {
	using T = std::underlying_type_t<ClearFlags>;
	return static_cast<ClearFlags>(static_cast<T>(a) & static_cast<T>(b));
}

constexpr bool HasFlag(ClearFlags value, ClearFlags flag) {
	return (value & flag) == flag;
}
