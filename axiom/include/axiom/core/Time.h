#pragma once

#include <chrono>

namespace axiom {
	class Time {
	public:
		static float GetTime();
		static float GetDeltaTime();
	};
}