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

        UUID Raw() const { return m_uuid; }

        bool IsValid() const { return m_uuid != UUID(); }

        std::string ToString() const {
            std::ostringstream oss;
            oss << m_uuid.ToHexString();
            return oss.str();
        }

        static TaggedUUID Generate() {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<uint64_t> dis;
            return TaggedUUID(dis(gen), dis(gen));
        }

        static TaggedUUID FromString(const std::string &value) {
            return TaggedUUID(UUID(value));
        }

        bool operator==(const TaggedUUID &) const = default;
        bool operator!=(const TaggedUUID &) const = default;

      private:
        UUID m_uuid;
    };

    struct AssetTag {};
    struct EntityTag {};

    using AssetID = TaggedUUID<AssetTag>;
    using EntityID = TaggedUUID<EntityTag>;

} // namespace axiom

namespace std {
    template <typename Tag> struct hash<axiom::TaggedUUID<Tag>> {
        size_t operator()(const axiom::TaggedUUID<Tag> &id) const {
            return std::hash<axiom::UUID>{}(id.Raw());
        }
    };
} // namespace std