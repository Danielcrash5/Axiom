/**
 * @file axiom/include/axiom/renderer/rhi/Backends.h
 * @brief Declares the public interface for the backends.
 */

#pragma once
#include <memory>
#include <span>
#include "IRHIBackend.h"

namespace axiom::renderer::rhi {

// Primaeres Desktop-Backend. Rueckgabetyp ist reines IRHIBackend - keine
// Vulkan-Typen tauchen in der Public-API auf.
/**
 * @brief Creates vulkan backend.
 * @param requiredInstanceExtensions The required instance extensions used by this operation.
 * @return The result of the operation.
 */
[[nodiscard]] RHIResult<std::unique_ptr<IRHIBackend>> createVulkanBackend(
    std::span<const char* const> requiredInstanceExtensions = {});

// Folgt in Phase 11, ausschliesslich fuer Web-Export-Target (Browser/WASM).
// [[nodiscard]] RHIResult<std::unique_ptr<IRHIBackend>> createWebGPUBackend();

} // namespace axiom::renderer::rhi
