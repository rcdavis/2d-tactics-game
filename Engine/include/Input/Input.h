#pragma once

#include "Input/KeyCodes.h"

struct PlatformEvent;

namespace Input {
	void BeginFrame();

	void HandleEvent(const PlatformEvent& event);

	bool IsKeyPressed(KeyCode key);
	bool IsKeyReleased(KeyCode key);
	bool IsKeyDown(KeyCode key);
}
