#pragma once

#include <memory>

#include "Asset.h"

namespace axiom {
class AssetLoader {
  public:
    static std::shared_ptr<Asset> Load(AssetID id);
};
} // namespace axiom