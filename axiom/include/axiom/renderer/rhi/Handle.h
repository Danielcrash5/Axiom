#pragma once
#include <cstdint>

namespace axiom::renderer::rhi {

    template <typename Tag>
    struct Handle {
        uint32_t index = 0;
        uint32_t generation = 0;

        constexpr Handle() = default;
        constexpr Handle(uint32_t index, uint32_t generation) noexcept
            : index(index), generation(generation) {
        }

        [[nodiscard]] constexpr bool valid() const noexcept {
            return generation != 0;
        }
        friend constexpr bool operator==(const Handle&, const Handle&) = default;
    };

    struct BufferTag {
    };
    struct TextureTag {
    };
    struct PipelineTag {
    };
    struct BindGroupTag {
    };
    struct SamplerTag {
    };

    using BufferHandle = Handle<BufferTag>;
    using TextureHandle = Handle<TextureTag>;
    using PipelineHandle = Handle<PipelineTag>;
    using BindGroupHandle = Handle<BindGroupTag>;
    using SamplerHandle = Handle<SamplerTag>;

} // namespace axiom::renderer::rhi