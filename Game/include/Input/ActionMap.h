#pragma once

#include <array>
#include <vector>
#include <variant>
#include <filesystem>

#include "Input/KeyCodes.h"

enum class Action : uint8_t {
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	Select,
	Cancel,
	OpenStatsMenu,
	Pause,
	Count
};

const char* ActionToString(Action action);
Action StringToAction(const char* const actionName);

struct KeyBinding {
	KeyCode key = KeyCode::Unknown;
};

// TODO: Add Gamepad support
using InputBinding = std::variant<KeyBinding>;

class ActionMap {
public:
	ActionMap() = default;

	void Bind(Action action, InputBinding binding);
	void ClearBindings(Action action);

	/**
	 * Updates the current state of all actions based on their bindings.
	 * This should be called once per frame after all events have been processed.
	 */
	void Update();

	bool IsDown(Action action) const;
	bool IsPressed(Action action) const;
	bool IsReleased(Action action) const;

	bool SaveBindings(const std::filesystem::path& filePath) const;

	static ActionMap LoadBindings(const std::filesystem::path& filePath);
	static ActionMap CreateDefault();

private:
	static float EvaluateBinding(const InputBinding& binding);

private:
	std::array<std::vector<InputBinding>, static_cast<size_t>(Action::Count)> mBindings;
	std::array<float, static_cast<size_t>(Action::Count)> mPrev;
	std::array<float, static_cast<size_t>(Action::Count)> mCur;
};
