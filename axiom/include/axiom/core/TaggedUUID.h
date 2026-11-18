/**
 * @file axiom/include/axiom/core/TaggedUUID.h
 * @brief Declares the public interface for the tagged UUID.
 */

// TypedUUID.h
#pragma once

#include "axiom/core/UUID.h"

#include <random>
#include <sstream>
#include <string>

namespace axiom {

    template <typename Tag> class TaggedUUID {
      public:
        TaggedUUID() : m_uuid() {}
        explicit TaggedUUID(UUID id) : m_uuid(id) {}
        TaggedUUID(uint64_t hi, uint64_t lo) : m_uuid(hi, lo) {}

        UUID raw() const { return m_uuid; }

        bool isValid() const { return m_uuid != UUID(); }

        std::string toString() const {
            std::ostringstream oss;
            oss << m_uuid.toHexString();
            return oss.str();
        }

        static TaggedUUID generate() {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<uint64_t> dis;
            return TaggedUUID(dis(gen), dis(gen));
        }

        static TaggedUUID fromString(const std::string &value) {
            return TaggedUUID(UUID(value));
        }

        bool operator==(const TaggedUUID &) const = default;
        bool operator!=(const TaggedUUID &) const = default;

      private:
        UUID m_uuid;
    };

    /** @brief Stores the data for the asset tag. */
    struct AssetTag {};
    /** @brief Stores the data for the entity tag. */
    struct EntityTag {};

    /**
     * @brief Provides the asset ID type alias.
     */
    using AssetID = TaggedUUID<AssetTag>;
    /**
     * @brief Provides the entity ID type alias.
     */
    using EntityID = TaggedUUID<EntityTag>;

} // namespace axiom

namespace std {
    template <typename Tag> struct hash<axiom::TaggedUUID<Tag>> {
        size_t operator()(const axiom::TaggedUUID<Tag> &id) const {
            return std::hash<axiom::UUID>{}(id.raw());
        }
    };
} // namespace std