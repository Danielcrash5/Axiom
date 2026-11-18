/**
 * @file axiom/include/axiom/renderer/rhi/BindGroup.h
 * @brief Declares the public interface for the bind group.
 */

#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include "Handle.h"
#include "ShaderDesc.h"

namespace axiom::renderer::rhi {

/** @brief Stores the data for the bind group layout tag. */
struct BindGroupLayoutTag {};
/**
 * @brief Provides the bind group layout handle type alias.
 */
using BindGroupLayoutHandle = Handle<BindGroupLayoutTag>;

/**
 * @brief Defines the available binding type values.
 * @tparam BindingType Template type used by this type.
 */
enum class BindingType { uniformBuffer, storageBuffer, sampledTexture, sampler };

/** @brief Stores the data for the bind group layout entry. */
struct BindGroupLayoutEntry {
    /** @brief Stores the binding. */
    uint32_t binding = 0;
    /** @brief Stores the type. */
    BindingType type = BindingType::uniformBuffer;
    /** @brief Stores the visibility. */
    ShaderStage visibility = ShaderStage::none;

    // Fuer Bindless (Design-Doc Abschnitt 6.1): variable-length Array-Binding
    // statt einem einzelnen Descriptor. Vulkan: VARIABLE_DESCRIPTOR_COUNT +
    // PARTIALLY_BOUND_BIT. WebGPU-Fallback (Phase 11): fester Slot-Pool.
    /** @brief Stores the bindless. */
    bool bindless = false;
    /** @brief Stores the bindless max count. */
    uint32_t bindlessMaxCount = 0; // nur relevant wenn bindless == true
};

/** @brief Stores the data for the bind group layout desc. */
struct BindGroupLayoutDesc {
    /** @brief Stores the entries. */
    std::vector<BindGroupLayoutEntry> entries;
};

/** @brief Stores the data for the bind group entry. */
struct BindGroupEntry {
    /** @brief Stores the binding. */
    uint32_t binding = 0;
    /** @brief Stores the array element. */
    uint32_t arrayElement = 0; // fuer bindless Bindings: Ziel-Slot im Array
    /** @brief Stores the buffer. */
    std::optional<BufferHandle> buffer;
    /** @brief Stores the texture. */
    std::optional<TextureHandle> texture;
    /** @brief Stores the sampler. */
    std::optional<SamplerHandle> sampler;
};

/** @brief Stores the data for the bind group desc. */
struct BindGroupDesc {
    /** @brief Stores the layout. */
    BindGroupLayoutHandle layout;
    /** @brief Stores the entries. */
    std::vector<BindGroupEntry> entries;

    // Nur relevant, wenn layout einen bindless-Eintrag hat: tatsaechliche
    // Array-Groesse bei der Allokation (kann <= bindlessMaxCount sein).
    /** @brief Stores the bindless count. */
    std::optional<uint32_t> bindlessCount;
};

} // namespace axiom::renderer::rhi
