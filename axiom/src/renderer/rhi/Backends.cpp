#include <axiom/renderer/rhi/Backends.h>
#include <axiom/renderer/vulkan/VulkanBackend.h>

namespace axiom::renderer::rhi {

RHIResult<std::unique_ptr<IRHIBackend>> createVulkanBackend(
    std::span<const char* const> requiredInstanceExtensions) {
    auto result = vulkan::VulkanBackend::create(requiredInstanceExtensions);
    if (!result) {
        return std::unexpected(result.error());
    }
    // impliziter Upcast unique_ptr<VulkanBackend> -> unique_ptr<IRHIBackend>
    return std::unique_ptr<IRHIBackend>(std::move(*result));
}

} // namespace axiom::renderer::rhi
