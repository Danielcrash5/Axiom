#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace axiom {

    enum class AssetTypeId : uint32_t {
        // Core types
        Texture = 0,
        Mesh = 1,
        Material = 2,
        Scene = 3,
        Prefab = 4,
        Script = 5,
        SoundEffect = 6,
        Music = 7,
        Font = 8,

        // Extensible - weitere Typen später
        Custom = 0xFFFF0000,
    };

    // 128-Bit eindeutige Asset-Identität
    struct TypedUUID {
        uint64_t high = 0;
        uint64_t low = 0;
        AssetTypeId type = AssetTypeId::Texture;

        TypedUUID() = default;
        TypedUUID(uint64_t h, uint64_t l, AssetTypeId t)
            : high(h), low(l), type(t) {}

        bool operator==(const TypedUUID &other) const {
            return high == other.high && low == other.low && type == other.type;
        }

        bool operator!=(const TypedUUID &other) const { return !(*this == other); }

        bool IsValid() const { return high != 0 || low != 0; }

        std::string ToString() const;

        static TypedUUID Generate(AssetTypeId type);
        static TypedUUID FromString(const std::string &str);
    };

} // namespace axiom

namespace std {
    template <> struct hash<axiom::TypedUUID> {
        size_t operator()(const axiom::TypedUUID &uuid) const {
            return std::hash<uint64_t>()(uuid.high) ^
                   (std::hash<uint64_t>()(uuid.low) << 1) ^
                   (std::hash<uint32_t>()(static_cast<uint32_t>(uuid.type)) << 2);
        }
    };
} // namespace std
