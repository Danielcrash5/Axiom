#include "axiom/core/Time.h"
#include <chrono>

double Time::s_Time = 0.0f;
double Time::s_LastTime = 0.0f;
double Time::s_DeltaTime = 0.0f;

void Time::Update() {
    using clock = std::chrono::high_resolution_clock;

    static auto startTime = clock::now();
    auto now = clock::now();

    std::chrono::duration<double> elapsed = now - startTime;
    s_Time = elapsed.count();

    s_DeltaTime = s_Time - s_LastTime;
    s_LastTime = s_Time;
}