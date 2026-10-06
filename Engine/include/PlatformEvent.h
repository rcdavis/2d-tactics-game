#pragma once

#include <cstdint>

struct PlatformEvent {
	enum class Type : uint8_t {
		WindowPixelSizeChanged,
		KeyDown,
		KeyUp,
		Quit,
		Count
	};

	Type type = Type::Count;

	union {
		struct {
			uint16_t width;
			uint16_t height;
		} windowSize;

		struct {
			uint16_t scancode;
		} key;
	};
};
