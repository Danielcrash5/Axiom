#pragma once

#include <memory>
#include <unordered_map>

#include "Asset.h"
#include "AssetLoader.h"

namespace axiom {
class AssetManager {
  public:
    template <typename T> static std::shared_ptr<T> Get(AssetID id) {
        auto it = s_Cache.find(id);

        if (it != s_Cache.end())
            return std::static_pointer_cast<T>(it->second);

        auto asset = AssetLoader::Load(id);

        s_Cache[id] = asset;

        return std::static_pointer_cast<T>(asset);
    }

  private:
    static inline std::unordered_map<AssetID, std::shared_ptr<Asset>> s_Cache;
};
} // namespace axiom