#pragma once
#include <memory>
#include <span>
#include <cstddef>
#include "RHITypes.h"
#include "CommandList.h"
#include "PipelineDesc.h"
#include "BindGroup.h"
#include "Sampler.h"

namespace axiom::renderer::rhi {

class IRHIBackend {
public:
    virtual ~IRHIBackend() = default;

    // --- Phase 1: Buffers/Textures ---
    [[nodiscard]] virtual RHIResult<BufferHandle> createBuffer(const BufferDesc&) = 0;
    [[nodiscard]] virtual RHIResult<TextureHandle> createTexture(const TextureDesc&) = 0;
    virtual void destroyBuffer(BufferHandle) = 0;
    virtual void destroyTexture(TextureHandle) = 0;

    [[nodiscard]] virtual RHIResult<void> uploadBufferData(
        BufferHandle target, uint64_t offset, std::span<const std::byte> data) = 0;
    [[nodiscard]] virtual RHIResult<void> uploadTextureData(
        TextureHandle target, const TextureUploadDesc& uploadDesc,
        std::span<const std::byte> data) = 0;

    // --- Phase 2: CommandList ---
    [[nodiscard]] virtual std::unique_ptr<CommandList> createCommandList() = 0;
    virtual void submit(CommandList&) = 0;

    // --- Fenster-Integration ---
    // Opake Handles nach aussen (void*), damit hier kein <vulkan/vulkan.h>
    // in der Public-API auftaucht. Intern: VkInstance / VkSurfaceKHR.
    [[nodiscard]] virtual void* nativeInstanceHandle() const = 0;
    [[nodiscard]] virtual RHIResult<void> attachSurface(void* nativeSurfaceHandle) = 0;

    // --- Phase 3: Pipelines/BindGroups/Sampler ---
    [[nodiscard]] virtual RHIResult<PipelineHandle> createPipeline(const PipelineDesc&) = 0;
    virtual void destroyPipeline(PipelineHandle) = 0;

    [[nodiscard]] virtual RHIResult<BindGroupLayoutHandle> createBindGroupLayout(const BindGroupLayoutDesc&) = 0;
    [[nodiscard]] virtual RHIResult<BindGroupHandle> createBindGroup(const BindGroupDesc&) = 0;
    // Aktualisiert einzelne Bindings einer bereits erzeugten BindGroup ohne
    // Neuerzeugung - noetig fuer Bindless-Slot-Updates (BindlessTextureHeap)
    // und generell fuer Streaming-Texturen.
    [[nodiscard]] virtual RHIResult<void> updateBindGroup(
        BindGroupHandle target, std::span<const BindGroupEntry> entries) = 0;
    virtual void destroyBindGroupLayout(BindGroupLayoutHandle) = 0;
    virtual void destroyBindGroup(BindGroupHandle) = 0;

    [[nodiscard]] virtual RHIResult<SamplerHandle> createSampler(const SamplerDesc&) = 0;
    virtual void destroySampler(SamplerHandle) = 0;
};

} // namespace axiom::renderer::rhi
