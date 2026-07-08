#pragma once
#include <memory>
#include "RHITypes.h"
#include "CommandList.h"

namespace axiom::renderer::rhi {

    class IRHIBackend {
    public:
        virtual ~IRHIBackend() = default;

        [[nodiscard]] virtual RHIResult<BufferHandle> createBuffer(const BufferDesc&) = 0;
        [[nodiscard]] virtual RHIResult<TextureHandle> createTexture(const TextureDesc&) = 0;

        virtual void destroyBuffer(BufferHandle) = 0;
        virtual void destroyTexture(TextureHandle) = 0;

        [[nodiscard]] virtual std::unique_ptr<CommandList> createCommandList() = 0;
        virtual void submit(CommandList&) = 0;
    };

} // namespace axiom::renderer::rhi