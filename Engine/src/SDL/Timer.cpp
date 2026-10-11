#include "Timer.h"

#include "SDL3/SDL_timer.h"

void Timer::Init() {
	mLastTime = SDL_GetTicks();
}

float Timer::GetElapsedTime() {
	const uint64_t currentTime = SDL_GetTicks();
	const float elapsedTime = static_cast<float>(currentTime - mLastTime) / 1'000.0f;
	mLastTime = currentTime;
	return elapsedTime;
}
