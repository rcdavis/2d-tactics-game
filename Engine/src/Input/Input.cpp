#include "Input/Input.h"

#include <array>

#include "PlatformEvent.h"

namespace Input {
	static std::array<bool, (uint16_t)KeyCode::Count> s_PrevKey{};
	static std::array<bool, (uint16_t)KeyCode::Count> s_CurrKey{};

	void BeginFrame() {
		s_PrevKey = s_CurrKey;
	}

	void HandleEvent(const PlatformEvent& event) {
		if (event.type == PlatformEvent::Type::KeyDown) {
			const uint16_t index = (uint16_t)event.key.scancode;
			s_CurrKey[index] = true;
		} else if (event.type == PlatformEvent::Type::KeyUp) {
			const uint16_t index = (uint16_t)event.key.scancode;
			s_CurrKey[index] = false;
		}
	}

	bool IsKeyPressed(KeyCode key) {
		const uint16_t index = (uint16_t)key;
		return s_CurrKey[index] && !s_PrevKey[index];
	}

	bool IsKeyReleased(KeyCode key) {
		const uint16_t index = (uint16_t)key;
		return !s_CurrKey[index] && s_PrevKey[index];
	}

	bool IsKeyDown(KeyCode key) {
		const uint16_t index = (uint16_t)key;
		return s_CurrKey[index];
	}
}
