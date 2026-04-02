// RenderStateCache.h
#pragma once
#include <cstring>
#include "RenderState.h"

class RenderStateCache {
public:
    RenderState Current {};

    bool SetIfChanged(const RenderState& state) {
        if (memcmp(&Current, &state, sizeof(RenderState)) == 0)
            return false;

        Current = state;
        return true;
    }
};