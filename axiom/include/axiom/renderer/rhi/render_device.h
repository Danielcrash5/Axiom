#pragma once

#include "../resources/texture.h"
#include "../resources/buffer.h"
#include "../resources/sampler.h"
#include "../resources/pipeline.h"

namespace axiom {
    class RenderDevice {
    public:

        virtual ~RenderDevice() = default;

        virtual TextureHandle create_texture(
            const TextureDesc& desc) = 0;

        virtual BufferHandle create_buffer(
            const BufferDesc& desc) = 0;

        virtual SamplerHandle create_sampler(
            const SamplerDesc& desc) = 0;

        virtual PipelineHandle create_pipeline(
            const PipelineDesc& desc) = 0;
    };
}