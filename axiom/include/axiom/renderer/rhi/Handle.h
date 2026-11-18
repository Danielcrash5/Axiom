/**
 * @file axiom/include/axiom/renderer/rhi/Handle.h
 * @brief Declares the public interface for the handle.
 */

#pragma once
#include <cstdint>

namespace axiom::renderer::rhi {

/**
 * @brief Stores the data for the handle.
 * @tparam Tag Template type used by this type.
 */
template <typename Tag>
struct Handle {
    /** @brief Stores the index. */
    uint32_t index = 0;
    /** @brief Stores the generation. */
    uint32_t generation = 0;

    /**
     * @brief Creates an instance of handle.
     */
    constexpr Handle() = default;
    /**
     * @brief Creates an instance of handle.
     * @param index The index used by this operation.
     * @param generation The generation used by this operation.
     */
    constexpr Handle(uint32_t index, uint32_t generation) noexcept
        : index(index), generation(generation) {}

    /**
     * @brief Performs the valid operation.
     * @return The result of the operation.
     */
    [[nodiscard]] constexpr bool valid() const noexcept { return generation != 0; }
    friend constexpr bool operator==(const Handle&, const Handle&) = default;
};

/** @brief Stores the data for the buffer tag. */
struct BufferTag {};
/** @brief Stores the data for the texture tag. */
struct TextureTag {};
/** @brief Stores the data for the pipeline tag. */
struct PipelineTag {};
/** @brief Stores the data for the bind group tag. */
struct BindGroupTag {};
/** @brief Stores the data for the sampler tag. */
struct SamplerTag {};
/** @brief Stores the data for the surface tag. */
struct SurfaceTag {};
/** @brief Stores the data for the swapchain tag. */
struct SwapchainTag {};

/**
 * @brief Provides the buffer handle type alias.
 */
using BufferHandle     = Handle<BufferTag>;
/**
 * @brief Provides the texture handle type alias.
 */
using TextureHandle    = Handle<TextureTag>;
/**
 * @brief Provides the pipeline handle type alias.
 */
using PipelineHandle   = Handle<PipelineTag>;
/**
 * @brief Provides the bind group handle type alias.
 */
using BindGroupHandle  = Handle<BindGroupTag>;
/**
 * @brief Provides the sampler handle type alias.
 */
using SamplerHandle    = Handle<SamplerTag>;
/**
 * @brief Provides the surface handle type alias.
 */
using SurfaceHandle    = Handle<SurfaceTag>;
/**
 * @brief Provides the swapchain handle type alias.
 */
using SwapchainHandle  = Handle<SwapchainTag>;

} // namespace axiom::renderer::rhi
