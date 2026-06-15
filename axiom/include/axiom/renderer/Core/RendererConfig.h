#pragma once

#include "RendererAPI.h"

namespace axiom {
    struct RendererConfig {
        RendererAPI API = RendererAPI::Vulkan;

        bool Validation = true;
    };
}