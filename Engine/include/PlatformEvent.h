#pragma once

#include <cstdint>

struct PlatformEvent {
	enum class Type : uint8_t {
		Quit,
		Count
	};

    Type type = Type::Count;
};
