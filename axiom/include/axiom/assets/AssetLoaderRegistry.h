#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "AssetType.h"

namespace axiom {
    // Legacy compatibility layer: older code still registers a loader by AssetType,
    // but the canonical manager stores loaders by AssetTypeId.
    using AssetLoaderFn = std::function<void *(const std::vector<uint8_t> &, size_t &)>;

    class AssetLoaderRegistry {
      public:
        static void RegisterLoader(AssetTypeId type, AssetLoaderFn loader);
        static const AssetLoaderFn *Find(AssetTypeId type);

      private:
        static inline std::unordered_map<AssetTypeId, AssetLoaderFn> s_Loaders;
    };

} // namespace axiom
