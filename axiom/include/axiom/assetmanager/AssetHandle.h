#pragma once
#include <cstdint>
#include <random>

namespace axiom {

    using AssetHandle = uint64_t;

    constexpr AssetHandle InvalidAssetHandle = 0;

    // Optional: Runtime UUID Generator
    inline AssetHandle GenerateAssetHandle() {
        static std::mt19937_64 rng(std::random_device {}());
        static std::uniform_int_distribution<uint64_t> dist;

        AssetHandle handle;
        do {
            handle = dist(rng);
        } while (handle == InvalidAssetHandle);

        return handle;
    }

}