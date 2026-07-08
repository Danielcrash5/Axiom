#include <axiom/renderer/rhi/Backends.h>
#include "../webgpu/WebGPUBackend.h"

namespace axiom::renderer::rhi {

    RHIResult<std::unique_ptr<IRHIBackend>> createWebGPUBackend() {
        auto result = webgpu::WebGPUBackend::create();
        if (!result) {
            return std::unexpected(result.error());
        }
        // impliziter Upcast unique_ptr<WebGPUBackend> -> unique_ptr<IRHIBackend>
        return std::unique_ptr<IRHIBackend>(std::move(*result));
    }

} // namespace axiom::renderer::rhi