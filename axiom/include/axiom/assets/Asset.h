#pragma once

#include "AssetID.h"
#include "AssetType.h"

namespace axiom {
class Asset {
  public:
    virtual ~Asset() = default;

    AssetID GetID() const { return m_ID; }

    AssetType GetType() const { return m_Type; }

  protected:
    AssetID m_ID{};
    AssetType m_Type = AssetType::Unknown;
};
} // namespace axiom