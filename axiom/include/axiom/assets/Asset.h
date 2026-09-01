#pragma once

#include "TypedUUID.h"
#include "AssetType.h"

namespace axiom {
    class Asset {
      public:
        virtual ~Asset() = default;

        TypedUUID GetID() const { return m_ID; }

        AssetType GetType() const { return m_Type; }

      protected:
        TypedUUID m_ID{};
        AssetType m_Type = AssetType::Unknown;
    };
} // namespace axiom