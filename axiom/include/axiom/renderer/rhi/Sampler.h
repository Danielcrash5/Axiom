/**
 * @file axiom/include/axiom/renderer/rhi/Sampler.h
 * @brief Declares the public interface for the sampler.
 */

#pragma once

namespace axiom::renderer::rhi {

/**
 * @brief Defines the available filter mode values.
 * @tparam FilterMode Template type used by this type.
 */
enum class FilterMode { nearest, linear };
/**
 * @brief Defines the available address mode values.
 * @tparam AddressMode Template type used by this type.
 */
enum class AddressMode { repeat, clampToEdge, mirroredRepeat };

/** @brief Stores the data for the sampler desc. */
struct SamplerDesc {
    /** @brief Stores the min filter. */
    FilterMode minFilter = FilterMode::linear;
    /** @brief Stores the mag filter. */
    FilterMode magFilter = FilterMode::linear;
    /** @brief Stores the address mode u. */
    AddressMode addressModeU = AddressMode::repeat;
    /** @brief Stores the address mode v. */
    AddressMode addressModeV = AddressMode::repeat;
};

} // namespace axiom::renderer::rhi
