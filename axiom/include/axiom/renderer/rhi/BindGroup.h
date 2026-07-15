#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include "Handle.h"
#include "ShaderDesc.h"

namespace axiom::renderer::rhi {

struct BindGroupLayoutTag {};
using BindGroupLayoutHandle = Handle<BindGroupLayoutTag>;

enum class BindingType { UniformBuffer, StorageBuffer, SampledTexture, Sampler };

struct BindGroupLayoutEntry {
    uint32_t binding = 0;
    BindingType type = BindingType::UniformBuffer;
    ShaderStage visibility = ShaderStage::None;

    // Fuer Bindless (Design-Doc Abschnitt 6.1): variable-length Array-Binding
    // statt einem einzelnen Descriptor. Vulkan: VARIABLE_DESCRIPTOR_COUNT +
    // PARTIALLY_BOUND_BIT. WebGPU-Fallback (Phase 11): fester Slot-Pool.
    bool bindless = false;
    uint32_t bindlessMaxCount = 0; // nur relevant wenn bindless == true
};

struct BindGroupLayoutDesc {
    std::vector<BindGroupLayoutEntry> entries;
};

struct BindGroupEntry {
    uint32_t binding = 0;
    uint32_t arrayElement = 0; // fuer bindless Bindings: Ziel-Slot im Array
    std::optional<BufferHandle> buffer;
    std::optional<TextureHandle> texture;
    std::optional<SamplerHandle> sampler;
};

struct BindGroupDesc {
    BindGroupLayoutHandle layout;
    std::vector<BindGroupEntry> entries;

    // Nur relevant, wenn layout einen bindless-Eintrag hat: tatsaechliche
    // Array-Groesse bei der Allokation (kann <= bindlessMaxCount sein).
    std::optional<uint32_t> bindlessCount;
};

} // namespace axiom::renderer::rhi
