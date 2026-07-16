#pragma once
#include <memory>
#include <span>
#include "IRHIBackend.h"

namespace axiom::renderer::rhi {

// Primaeres Desktop-Backend. Rueckgabetyp ist reines IRHIBackend - keine
// Vulkan-Typen tauchen in der Public-API auf.
[[nodiscard]] RHIResult<std::unique_ptr<IRHIBackend>> createVulkanBackend(
    std::span<const char* const> requiredInstanceExtensions = {});

// Folgt in Phase 11, ausschliesslich fuer Web-Export-Target (Browser/WASM).
// [[nodiscard]] RHIResult<std::unique_ptr<IRHIBackend>> createWebGPUBackend();

} // namespace axiom::renderer::rhi
