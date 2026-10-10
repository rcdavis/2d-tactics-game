#include "Input/ActionMap.h"

#include <fstream>

#include "Input/Input.h"
#include "Utils/Log.h"
#include "nlohmann/json.hpp"

static constexpr float PressedThreshold = 0.0f;

const char* ActionToString(Action action) {
	switch (action) {
		case Action::MoveLeft:  return "MoveLeft";
		case Action::MoveRight: return "MoveRight";
		case Action::MoveUp:    return "MoveUp";
		case Action::MoveDown:  return "MoveDown";
		case Action::Select:    return "Select";
		case Action::Cancel:    return "Cancel";
		case Action::Pause:     return "Pause";
		case Action::Count:     return "Count";
		default:                return "Unknown";
	}
}

Action StringToAction(const char* const actionName) {
	if (strcmp(actionName, "MoveLeft") == 0)  return Action::MoveLeft;
	if (strcmp(actionName, "MoveRight") == 0) return Action::MoveRight;
	if (strcmp(actionName, "MoveUp") == 0)    return Action::MoveUp;
	if (strcmp(actionName, "MoveDown") == 0)  return Action::MoveDown;
	if (strcmp(actionName, "Select") == 0)    return Action::Select;
	if (strcmp(actionName, "Cancel") == 0)    return Action::Cancel;
	if (strcmp(actionName, "Pause") == 0)     return Action::Pause;
	if (strcmp(actionName, "Count") == 0)     return Action::Count;
	return Action::Count; // Default to Count if unknown
}

void ActionMap::Bind(Action action, Binding binding) {
	mBindings[static_cast<size_t>(action)].push_back(binding);
}

void ActionMap::ClearBindings(Action action) {
	mBindings[static_cast<size_t>(action)].clear();
}

void ActionMap::Update() {
	mPrev = mCur;
	for (uint8_t i = 0; i < static_cast<uint8_t>(Action::Count); ++i) {
		float value = 0.0f;
		for (const auto& binding : mBindings[i]) {
			value = std::max(value, EvaluateBinding(binding));
		}
		mCur[i] = value;
	}
}

bool ActionMap::IsDown(Action action) const {
	return mCur[static_cast<size_t>(action)] > PressedThreshold;
}

bool ActionMap::IsPressed(Action action) const {
	const size_t index = static_cast<size_t>(action);
	return mCur[index] > PressedThreshold && mPrev[index] <= PressedThreshold;
}

bool ActionMap::IsReleased(Action action) const {
	const size_t index = static_cast<size_t>(action);
	return mCur[index] <= PressedThreshold && mPrev[index] > PressedThreshold;
}

float ActionMap::EvaluateBinding(const Binding& binding) {
	if (const KeyBinding* key = std::get_if<KeyBinding>(&binding)) {
		return Input::IsKeyDown(key->key) ? 1.0f : 0.0f;
	}

	return 0.0f;
}

bool ActionMap::SaveBindings(const std::filesystem::path& filePath) const {
	std::filesystem::create_directories(filePath.parent_path());

	const std::filesystem::path tempFilePath = filePath.string() + ".tmp";
	std::ofstream file(tempFilePath, std::ios::trunc);
	if (!file) {
		LOG_ERROR("Failed to open file for saving bindings: {}", tempFilePath.string());
		return false;
	}

	nlohmann::json bindings = nlohmann::json::object();
	for (uint8_t i = 0; i < static_cast<uint8_t>(Action::Count); ++i) {
		const char* const actionName = ActionToString(static_cast<Action>(i));
		nlohmann::json bindingsForActionJson = nlohmann::json::array();

		for (const auto& binding : mBindings[i]) {
			if (const KeyBinding* key = std::get_if<KeyBinding>(&binding)) {
				bindingsForActionJson.push_back({
					{"type", "key"},
					{"key", static_cast<int>(key->key)}
				});
			}
		}

		bindings[actionName] = bindingsForActionJson;
	}

	const nlohmann::json root = {
		{"version", 1},
		{"bindings", bindings},
	};

	file << root.dump(2);
	file.close();

	std::filesystem::rename(tempFilePath, filePath);

	return true;
}

ActionMap ActionMap::CreateDefault() {
	ActionMap map;

	map.Bind(Action::MoveLeft, KeyBinding{KeyCode::Left});

	map.Bind(Action::MoveRight, KeyBinding{KeyCode::Right});

	map.Bind(Action::MoveUp, KeyBinding{KeyCode::Up});

	map.Bind(Action::MoveDown, KeyBinding{KeyCode::Down});

	map.Bind(Action::Select, KeyBinding{KeyCode::Enter});
	map.Bind(Action::Select, KeyBinding{KeyCode::F});

	map.Bind(Action::Cancel, KeyBinding{KeyCode::D});

	map.Bind(Action::Pause, KeyBinding{KeyCode::Escape});

	return map;
}
