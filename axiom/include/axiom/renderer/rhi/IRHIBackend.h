#pragma once
#include "BindGroup.h"
#include "CommandList.h"
#include "PipelineDesc.h"
#include "RHITypes.h"
#include <cstddef>
#include <memory>
#include <span>

namespace axiom::renderer::rhi {

class IRHIBackend {
  public:
    virtual ~IRHIBackend() = default;

    [[nodiscard]] virtual RHIResult<BufferHandle>
    createBuffer(const BufferDesc &) = 0;
    [[nodiscard]] virtual RHIResult<TextureHandle>
    createTexture(const TextureDesc &) = 0;

    virtual void destroyBuffer(BufferHandle) = 0;
    virtual void destroyTexture(TextureHandle) = 0;

    [[nodiscard]] virtual RHIResult<void>
    uploadBufferData(BufferHandle target, uint64_t offset,
                     std::span<const std::byte> data) = 0;

    [[nodiscard]] virtual RHIResult<void>
    uploadTextureData(TextureHandle target, const TextureUploadDesc &uploadDesc,
                      std::span<const std::byte> data) = 0;

    [[nodiscard]] virtual std::unique_ptr<CommandList> createCommandList() = 0;
    virtual void submit(CommandList &) = 0;

    [[nodiscard]] virtual RHIResult<PipelineHandle>
    createPipeline(const PipelineDesc &) = 0;
    virtual void destroyPipeline(PipelineHandle) = 0;

    [[nodiscard]] virtual RHIResult<BindGroupLayoutHandle>
    createBindGroupLayout(const BindGroupLayoutDesc &) = 0;
    [[nodiscard]] virtual RHIResult<BindGroupHandle>
    createBindGroup(const BindGroupDesc &) = 0;
    virtual void destroyBindGroupLayout(BindGroupLayoutHandle) = 0;
    virtual void destroyBindGroup(BindGroupHandle) = 0;
};

} // namespace axiom::renderer::rhi