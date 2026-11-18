/**
 * @file axiom/include/axiom/renderer/rhi/IRHIBackend.h
 * @brief Declares the public interface for the irhi backend.
 */

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

/**
 * @brief Defines the interface for the RHI backend.
 * @tparam IRHIBackend Template type used by this type.
 */
class IRHIBackend {
public:
    /**
     * @brief Releases resources owned by IRHIBackend.
     */
    virtual ~IRHIBackend() = default;

    // --- Phase 1: Buffers/Textures ---
    /**
     * @brief Creates buffer.
     * @param BufferDesc The buffer desc used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<BufferHandle> createBuffer(const BufferDesc&) = 0;
    /**
     * @brief Creates texture.
     * @param TextureDesc The texture desc used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<TextureHandle> createTexture(const TextureDesc&) = 0;
    /**
     * @brief Destroys the buffer and releases its resources.
     */
    virtual void destroyBuffer(BufferHandle) = 0;
    /**
     * @brief Destroys the texture and releases its resources.
     */
    virtual void destroyTexture(TextureHandle) = 0;

    /**
     * @brief Uploads data to a buffer.
     * @param target The target used by this operation.
     * @param offset The offset used by this operation.
     * @param data The data used by this operation.
     */
    [[nodiscard]] virtual RHIResult<void> uploadBufferData(
        BufferHandle target, uint64_t offset, std::span<const std::byte> data) = 0;
    /**
     * @brief Uploads data to a texture.
     * @param target The target used by this operation.
     * @param uploadDesc The upload desc used by this operation.
     * @param data The data used by this operation.
     */
    [[nodiscard]] virtual RHIResult<void> uploadTextureData(
        TextureHandle target, const TextureUploadDesc& uploadDesc,
        std::span<const std::byte> data) = 0;

    // --- Phase 2: CommandList ---
    /**
     * @brief Creates command list.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual std::unique_ptr<CommandList> createCommandList() = 0;
    /**
     * @brief Submits the collected work for execution.
     */
    virtual void submit(CommandList&) = 0;

    // --- Fenster-Integration ---
    // Opake Handles nach aussen (void*), damit hier kein <vulkan/vulkan.h>
    // in der Public-API auftaucht. Intern: VkInstance / VkSurfaceKHR.
    // Mehrere Surfaces gleichzeitig moeglich (Slot-Pool wie Buffers/Textures) -
    // noetig fuer ImGui-Viewports: jedes rausgedockte Panel-Fenster bekommt
    // eine eigene Surface + eigenen Swapchain (Phase 8).
    /**
     * @brief Returns the native instance handle.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual void* nativeInstanceHandle() const = 0;
    /**
     * @brief Creates surface.
     * @param nativeSurfaceHandle The native surface handle used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<SurfaceHandle> createSurface(void* nativeSurfaceHandle) = 0;
    /**
     * @brief Destroys the surface and releases its resources.
     */
    virtual void destroySurface(SurfaceHandle) = 0;

    // --- Phase 3: Pipelines/BindGroups/Sampler ---
    /**
     * @brief Creates pipeline.
     * @param PipelineDesc The pipeline desc used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<PipelineHandle> createPipeline(const PipelineDesc&) = 0;
    /**
     * @brief Destroys the pipeline and releases its resources.
     */
    virtual void destroyPipeline(PipelineHandle) = 0;

    /**
     * @brief Creates bind group layout.
     * @param BindGroupLayoutDesc The bind group layout desc used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<BindGroupLayoutHandle> createBindGroupLayout(const BindGroupLayoutDesc&) = 0;
    /**
     * @brief Creates bind group.
     * @param BindGroupDesc The bind group desc used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<BindGroupHandle> createBindGroup(const BindGroupDesc&) = 0;
    // Aktualisiert einzelne Bindings einer bereits erzeugten BindGroup ohne
    // Neuerzeugung - noetig fuer Bindless-Slot-Updates (BindlessTextureHeap)
    // und generell fuer Streaming-Texturen.
    /**
     * @brief Updates bind group.
     * @param target The target used by this operation.
     * @param entries The entries used by this operation.
     */
    [[nodiscard]] virtual RHIResult<void> updateBindGroup(
        BindGroupHandle target, std::span<const BindGroupEntry> entries) = 0;
    /**
     * @brief Destroys the bind group layout.
     */
    virtual void destroyBindGroupLayout(BindGroupLayoutHandle) = 0;
    /**
     * @brief Destroys the bind group.
     */
    virtual void destroyBindGroup(BindGroupHandle) = 0;

    /**
     * @brief Creates sampler.
     * @param SamplerDesc The sampler desc used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<SamplerHandle> createSampler(const SamplerDesc&) = 0;
    /**
     * @brief Destroys the sampler.
     */
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
    /**
     * @brief Creates swapchain.
     * @param SwapchainDesc The swapchain desc used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<SwapchainHandle> createSwapchain(const SwapchainDesc&) = 0;
    /**
     * @brief Destroys the swapchain and releases its resources.
     */
    virtual void destroySwapchain(SwapchainHandle) = 0;

    // Erkennt VK_ERROR_OUT_OF_DATE_KHR/SUBOPTIMAL intern und rekonstruiert die
    // Swapchain automatisch anhand der aktuellen Surface-Capabilities, bevor
    // erneut versucht wird - Aufrufer muss Resize nicht selbst behandeln.
    /**
     * @brief Acquires the next image from the swapchain.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual RHIResult<AcquiredImage> acquireNextImage(SwapchainHandle) = 0;
    /**
     * @brief Presents the rendered image to the display.
     * @param imageIndex The image index used by this operation.
     */
    [[nodiscard]] virtual RHIResult<void> present(SwapchainHandle, uint32_t imageIndex) = 0;

    // Baut die Swapchain mit FIFO (true) bzw. MAILBOX/IMMEDIATE (false) neu -
    // synchron, wartet also einmal auf das Device (siehe VulkanBackend::
    // recreateSwapchainInternal). Nicht pro Frame aufrufen, nur bei
    // Nutzeraktion (Settings-Toggle).
    /**
     * @brief Sets swapchain vsync.
     * @param enabled The enabled used by this operation.
     */
    [[nodiscard]] virtual RHIResult<void> setSwapchainVsync(SwapchainHandle,
                                                            bool enabled) = 0;

    /**
     * @brief Returns the swapchain format.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual TextureFormat swapchainFormat(SwapchainHandle) const = 0;
    /**
     * @brief Returns the swapchain extent.
     * @return The result of the operation.
     */
    [[nodiscard]] virtual std::pair<uint32_t, uint32_t> swapchainExtent(SwapchainHandle) const = 0;
};

} // namespace axiom::renderer::rhi
