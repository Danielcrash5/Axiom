/**
 * @file axiom/include/axiom/renderer/BindlessTextureHeap.h
 * @brief Declares the public interface for the bindless texture heap.
 */

#pragma once
#include <cstdint>
#include <vector>
#include <axiom/renderer/rhi/IRHIBackend.h>

namespace axiom::renderer {

// Vulkan Descriptor Indexing als Primaerpfad (Design-Doc Abschnitt 6.1) -
// Device-Features (descriptorIndexing, runtimeDescriptorArray,
// partiallyBound) wurden schon in Phase 1 aktiviert. Kein Vulkan-Header
// hier - laeuft komplett ueber IRHIBackend::updateBindGroup.
/**
 * @brief Provides the bindless texture heap functionality.
 * @tparam BindlessTextureHeap Template type used by this type.
 */
class BindlessTextureHeap {
public:
    /** @brief Stores the k max textures. */
    static constexpr uint32_t kMaxTextures = 4096;

    /**
     * @brief Initializes the subsystem.
     * @param backend The backend used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> init(rhi::IRHIBackend& backend);
    /**
     * @brief Shuts down the subsystem.
     */
    void shutdown();

    /**
     * @brief Performs the allocate operation.
     * @param texture The texture used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::RHIResult<uint32_t> allocate(rhi::TextureHandle texture);
    /**
     * @brief Performs the free operation.
     * @param index The index used by this operation.
     */
    void free(uint32_t index);
    /**
     * @brief Updates the object state.
     * @param index The index used by this operation.
     * @param newTexture The new texture used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> update(uint32_t index, rhi::TextureHandle newTexture);

    /**
     * @brief Returns the layout.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::BindGroupLayoutHandle layout() const { return m_layout; }
    /**
     * @brief Binds group.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::BindGroupHandle bindGroup() const { return m_bindGroup; }
    /**
     * @brief Initializes ialized.
     * @return The result of the operation.
     */
    [[nodiscard]] bool initialized() const { return m_backend != nullptr && m_bindGroup.valid(); }
    /**
     * @brief Reports whether allocated.
     * @param index The index used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] bool isAllocated(uint32_t index) const;

private:
    /** @brief Stores the backend. */
    rhi::IRHIBackend* m_backend = nullptr;
    /** @brief Stores the layout. */
    rhi::BindGroupLayoutHandle m_layout;
    /** @brief Stores the bind group. */
    rhi::BindGroupHandle m_bindGroup;
    /** @brief Stores the slots. */
    std::vector<rhi::TextureHandle> m_slots;
    /** @brief Stores the slot used. */
    std::vector<bool> m_slotUsed;
    /** @brief Stores the free slots. */
    std::vector<uint32_t> m_freeSlots;
    /** @brief Stores the next slot. */
    uint32_t m_nextSlot = 0;
};

} // namespace axiom::renderer
