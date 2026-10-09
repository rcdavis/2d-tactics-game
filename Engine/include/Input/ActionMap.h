#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <variant>

#include "Input/KeyCodes.h"

enum class Action : uint8_t {
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	Select,
	Cancel,
	Pause,
	Count
};

struct KeyBinding {
	KeyCode key = KeyCode::Unknown;
};

// TODO: Add Gamepad support
using Binding = std::variant<KeyBinding>;

class ActionMap {
public:
    ActionMap() = default;

	void Bind(Action action, Binding binding);
	void ClearBindings(Action action);

	/**
	 * Updates the current state of all actions based on their bindings.
	 * This should be called once per frame after all events have been processed.
	 */
	void Update();

	bool IsDown(Action action) const;
	bool IsPressed(Action action) const;
	bool IsReleased(Action action) const;

	bool SaveBindings(const char* const filePath) const;

	static ActionMap CreateDefault();

private:
	static float EvaluateBinding(const Binding& binding);

private:
    std::array<std::vector<Binding>, static_cast<size_t>(Action::Count)> mBindings;
	std::array<float, static_cast<size_t>(Action::Count)> mPrev;
	std::array<float, static_cast<size_t>(Action::Count)> mCur;
};
