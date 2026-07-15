#pragma once

namespace axiom::renderer::rhi {

enum class FilterMode { Nearest, Linear };
enum class AddressMode { Repeat, ClampToEdge, MirroredRepeat };

struct SamplerDesc {
    FilterMode minFilter = FilterMode::Linear;
    FilterMode magFilter = FilterMode::Linear;
    AddressMode addressModeU = AddressMode::Repeat;
    AddressMode addressModeV = AddressMode::Repeat;
};

} // namespace axiom::renderer::rhi
