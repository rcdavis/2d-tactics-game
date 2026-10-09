#include "Input/ActionMap.h"

#include "Input/Input.h"

static constexpr float PressedThreshold = 0.0f;

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

bool ActionMap::SaveBindings(const char* const filePath) const {
	// Implement saving input bindings here
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
