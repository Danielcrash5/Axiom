#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/rhi/Backends.h>

namespace axiom::renderer {

rhi::RHIResult<void> Renderer::init(const WindowSurfaceDesc& windowDesc) {
    auto backendResult = rhi::createVulkanBackend(windowDesc.requiredInstanceExtensions);
    if (!backendResult) {
        return std::unexpected(backendResult.error());
    }
    m_backend = std::move(*backendResult);

    if (windowDesc.createSurface) {
        auto nativeSurfaceResult = windowDesc.createSurface(m_backend->nativeInstanceHandle());
        if (!nativeSurfaceResult) {
            return std::unexpected(nativeSurfaceResult.error());
        }

        auto surfaceResult = m_backend->createSurface(*nativeSurfaceResult);
        if (!surfaceResult) {
            return std::unexpected(surfaceResult.error());
        }
        m_mainSurface = *surfaceResult;
    }

    return {};
}

} // namespace axiom::renderer
