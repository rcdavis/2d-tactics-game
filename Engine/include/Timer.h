#pragma once

#include <cstdint>

class Timer {
public:
	Timer() = default;

	void Init();

	float GetElapsedTime();

private:
	uint64_t mLastTime = 0;
};
