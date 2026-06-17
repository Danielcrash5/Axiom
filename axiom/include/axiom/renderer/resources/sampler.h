#pragma once

namespace axiom {
    enum class FilterMode {
        Nearest,
        Linear
    };

    enum class AddressMode {
        Repeat,
        ClampToEdge,
        MirroredRepeat
    };

    struct SamplerDesc {
        FilterMode min_filter =
            FilterMode::Linear;

        FilterMode mag_filter =
            FilterMode::Linear;

        AddressMode address_u =
            AddressMode::Repeat;

        AddressMode address_v =
            AddressMode::Repeat;

        AddressMode address_w =
            AddressMode::Repeat;
    };
}