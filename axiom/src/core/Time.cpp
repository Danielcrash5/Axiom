/**
 * @file axiom/src/core/Time.cpp
 * @brief Implements the time module.
 */

#include "axiom/core/Time.h"
#include <chrono>

double axiom::Time::sTime = 0.0f;
double axiom::Time::sLastTime = 0.0f;
double axiom::Time::sDeltaTime = 0.0f;

void axiom::Time::update() {
    using clock = std::chrono::high_resolution_clock;

    static auto startTime = clock::now();
    auto now = clock::now();

    std::chrono::duration<double> elapsed = now - startTime;
    sTime = elapsed.count();

    sDeltaTime = sTime - sLastTime;
    sLastTime = sTime;
}