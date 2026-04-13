// RenderStateCache.h
#pragma once
#include <cstring>
#include "RenderState.h"

class RenderStateCache {
public:
    RenderState Current {};
    bool Initialized = false;

    bool SetIfChanged(const RenderState& state) {
        if (Initialized && memcmp(&Current, &state, sizeof(RenderState)) == 0)
            return false;

        Current = state;
        Initialized = true;
        return true;
    }
};
