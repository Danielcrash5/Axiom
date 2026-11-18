/**
 * @file axiom/include/axiom/assets/TypedUUID.h
 * @brief Declares the public interface for the typed UUID.
 */

#pragma once

#include "AssetType.h"
#include "axiom/core/UUID.h"

#include <cstdint>
#include <functional>
#include <random>
#include <sstream>
#include <string>

namespace axiom {
    /** @brief Associates a UUID with an asset type. */
    struct TypedUUID {
        /** @brief Stores the high. */
        uint64_t high = 0;
        /** @brief Stores the low. */
        uint64_t low = 0;
        /** @brief Stores the type. */
        AssetTypeId type = AssetTypeId::texture;

        /**
         * @brief Creates an instance of typed UUID.
         */
        TypedUUID() = default;
        /**
         * @brief Creates an instance of typed UUID.
         * @param h The h used by this operation.
         * @param l The l used by this operation.
         * @param t The t used by this operation.
         */
        TypedUUID(uint64_t h, uint64_t l, AssetTypeId t = AssetTypeId::texture)
            : high(h), low(l), type(t) {}

        /**
         * @brief Creates an instance of typed UUID.
         * @param uuid The UUID used by this operation.
         * @param t The t used by this operation.
         */
        explicit TypedUUID(const UUID &uuid, AssetTypeId t = AssetTypeId::texture)
            : high(uuid.hi()), low(uuid.lo()), type(t) {}

        bool operator==(const TypedUUID &other) const {
            return high == other.high && low == other.low && type == other.type;
        }

        bool operator!=(const TypedUUID &other) const { return !(*this == other); }

        /**
         * @brief Reports whether valid.
         * @return The result of the operation.
         */
        bool isValid() const { return high != 0 || low != 0; }

        /**
         * @brief Converts this value to UUID.
         * @return The result of the operation.
         */
        UUID toUuid() const { return UUID(high, low); }

        /**
         * @brief Converts this value to string.
         * @return The result of the operation.
         */
        std::string toString() const {
            std::ostringstream ss;
            ss << std::hex << std::uppercase << high << "-" << low << "-"
               << static_cast<uint32_t>(type);
            return ss.str();
        }

        /**
         * @brief Generates the requested data.
         * @param t The t used by this operation.
         * @return The result of the operation.
         */
        static TypedUUID generate(AssetTypeId t) {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<uint64_t> dis;
            return TypedUUID(dis(gen), dis(gen), t);
        }

        /**
         * @brief Creates a value from string data.
         * @param str The str used by this operation.
         * @return The result of the operation.
         */
        static TypedUUID fromString(const std::string &str) {
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

    /**
     * @brief Provides the asset UUID type alias.
     */
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
