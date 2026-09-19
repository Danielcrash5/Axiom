#pragma once
#include <memory>
#include <span>
#include <cstddef>
#include <utility>
#include "RHITypes.h"
#include "CommandList.h"
#include "PipelineDesc.h"
#include "BindGroup.h"
#include "Sampler.h"
#include "Swapchain.h"

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
    // Mehrere Surfaces gleichzeitig moeglich (Slot-Pool wie Buffers/Textures) -
    // noetig fuer ImGui-Viewports: jedes rausgedockte Panel-Fenster bekommt
    // eine eigene Surface + eigenen Swapchain (Phase 8).
    [[nodiscard]] virtual void* nativeInstanceHandle() const = 0;
    [[nodiscard]] virtual RHIResult<SurfaceHandle> createSurface(void* nativeSurfaceHandle) = 0;
    virtual void destroySurface(SurfaceHandle) = 0;

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

    // --- Swapchain/Present ---
    // Jedes Swapchain-Image bekommt EINEN stabilen TextureHandle bei
    // createSwapchain() (siehe Swapchain.h) - kein Handle-Churn pro Frame.
    // Vollstaendig synchron (kein Frames-in-Flight-Overlap): acquireNextImage()
    // blockiert per Fence, bis das Image tatsaechlich beschreibbar ist; submit()
    // blockiert schon vorher bis GPU-Fertigstellung - present() braucht daher
    // keine Wait-Semaphores mehr. Passt zum bisherigen synchronen Stil dieses
    // Backends (wie z.B. bei den Immediate-Uploads); Ueberlappung ist eine
    // spaetere Optimierung, kein Korrektheitsproblem.
    [[nodiscard]] virtual RHIResult<SwapchainHandle> createSwapchain(const SwapchainDesc&) = 0;
    virtual void destroySwapchain(SwapchainHandle) = 0;

    // Erkennt VK_ERROR_OUT_OF_DATE_KHR/SUBOPTIMAL intern und rekonstruiert die
    // Swapchain automatisch anhand der aktuellen Surface-Capabilities, bevor
    // erneut versucht wird - Aufrufer muss Resize nicht selbst behandeln.
    [[nodiscard]] virtual RHIResult<AcquiredImage> acquireNextImage(SwapchainHandle) = 0;
    [[nodiscard]] virtual RHIResult<void> present(SwapchainHandle, uint32_t imageIndex) = 0;

    [[nodiscard]] virtual TextureFormat swapchainFormat(SwapchainHandle) const = 0;
    [[nodiscard]] virtual std::pair<uint32_t, uint32_t> swapchainExtent(SwapchainHandle) const = 0;
};

} // namespace axiom::renderer::rhi
