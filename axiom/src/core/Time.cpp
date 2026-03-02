#include "axiom/core/Time.h"

namespace axiom {
	float Time::GetTime() {
		using clock = std::chrono::high_resolution_clock;
		static auto startTime = clock::now();
		auto currentTime = clock::now();
		std::chrono::duration<float> elapsed = currentTime - startTime;
		return elapsed.count();
	}

	float Time::GetDeltaTime() {
		static float lastTime = GetTime();
		float currentTime = GetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		return deltaTime;
	}
}