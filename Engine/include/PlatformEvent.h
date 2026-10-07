#pragma once

#include "Input/KeyCodes.h"

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
			KeyCode scancode;
		} key;
	};
};
