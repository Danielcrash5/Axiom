// TypedUUID.h
#pragma once
#include "axiom/core/UUID.h"

namespace axiom {

    template <typename Tag> class TypedUUID {
      public:
        TypedUUID() : m_uuid() {} // generiert neue UUID
        explicit TypedUUID(UUID id) : m_uuid(id) {}

        UUID Raw() const { return m_uuid; }

        bool operator==(const TypedUUID &) const = default;
        bool operator!=(const TypedUUID &) const = default;

      private:
        UUID m_uuid;
    };

    struct AssetTag {};
    struct EntityTag {};

    using AssetID = TypedUUID<AssetTag>;
    using EntityID = TypedUUID<EntityTag>;

} // namespace axiom

namespace std {
    template <typename Tag> struct hash<axiom::TypedUUID<Tag>> {
        size_t operator()(const axiom::TypedUUID<Tag> &id) const {
            return std::hash<axiom::UUID>{}(id.Raw());
        }
    };
} // namespace std