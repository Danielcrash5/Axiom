#pragma once

#include "AssetType.h"
#include "axiom/core/UUID.h"

#include <cstdint>
#include <functional>
#include <random>
#include <sstream>
#include <string>

namespace axiom {
    struct TypedUUID {
        uint64_t high = 0;
        uint64_t low = 0;
        AssetTypeId type = AssetTypeId::Texture;

        TypedUUID() = default;
        TypedUUID(uint64_t h, uint64_t l, AssetTypeId t = AssetTypeId::Texture)
            : high(h), low(l), type(t) {}

        explicit TypedUUID(const UUID &uuid, AssetTypeId t = AssetTypeId::Texture)
            : high(uuid.Hi()), low(uuid.Lo()), type(t) {}

        bool operator==(const TypedUUID &other) const {
            return high == other.high && low == other.low && type == other.type;
        }

        bool operator!=(const TypedUUID &other) const { return !(*this == other); }

        bool IsValid() const { return high != 0 || low != 0; }

        UUID ToUUID() const { return UUID(high, low); }

        std::string ToString() const {
            std::ostringstream ss;
            ss << std::hex << std::uppercase << high << "-" << low << "-"
               << static_cast<uint32_t>(type);
            return ss.str();
        }

        static TypedUUID Generate(AssetTypeId t) {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<uint64_t> dis;
            return TypedUUID(dis(gen), dis(gen), t);
        }

        static TypedUUID FromString(const std::string &str) {
            TypedUUID uuid;
            std::stringstream ss(str);
            char dash = 0;
            ss >> std::hex >> uuid.high >> dash >> uuid.low >> dash;
            uint32_t typeInt = 0;
            ss >> typeInt;
            uuid.type = static_cast<AssetTypeId>(typeInt);
            return uuid;
        }
    };

    using AssetUUID = TypedUUID;
    // Note: AssetID is defined in axiom/core/TypedUUID.h for ECS compatibility
    // Asset code should use AssetUUID (=TypedUUID) for asset identification
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
