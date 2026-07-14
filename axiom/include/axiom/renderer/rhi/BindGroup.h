#pragma once
#include "Handle.h"
#include "ShaderDesc.h"
#include <cstdint>
#include <optional>
#include <vector>

namespace axiom::renderer::rhi {
struct BindGroupLayoutTag {};
using BindGroupLayoutHandle = Handle<BindGroupLayoutTag>;

enum class BindingType {
    UniformBuffer,
    StorageBuffer,
    SampledTexture,
    Sampler
};

struct BindGroupLayoutDesc {
    std::vector<BindGroupLayoutEntry> entries;
};

struct BindGroupLayoutEntry {
    uint32_t binding = 0;
    BindingType type = BindingType::UniformBuffer;
    ShaderStage visibility = ShaderStage::None;

    // Für Bindless (Abschnitt 6.1): variable-length Array-Binding statt
    // einem einzelnen Descriptor. Vulkan: VARIABLE_DESCRIPTOR_COUNT +
    // PARTIALLY_BOUND_BIT. WebGPU-Fallback (Phase 11): fester Slot-Pool
    // in dieser Größe.
    bool bindless = false;
    uint32_t bindlessMaxCount = 0; // nur relevant wenn bindless == true
};

struct BindGroupEntry {
    uint32_t binding = 0;
    std::optional<BufferHandle> buffer;
    std::optional<TextureHandle> texture;
    std::optional<SamplerHandle> sampler;
};

struct BindGroupDesc {
    BindGroupLayoutHandle layout;
    std::vector<BindGroupEntry> entries;

    // Nur relevant, wenn layout einen bindless-Eintrag hat: tatsächliche
    // Array-Größe bei der Allokation (kann <= bindlessMaxCount sein).
    std::optional<uint32_t> bindlessCount;
};

} // namespace axiom::renderer::rhi