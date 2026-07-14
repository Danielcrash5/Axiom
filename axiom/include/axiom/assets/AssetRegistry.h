#pragma once

#include <unordered_map>

#include "AssetMetadata.h"

namespace axiom {
    class AssetRegistry {
      public:
        static AssetID Register(const std::string &path, AssetType type);

        static const AssetMetadata *Get(AssetID id);

        static AssetID Find(const std::string &path);

      private:
        static inline std::unordered_map<AssetID, AssetMetadata> s_Metadata;
        static inline std::unordered_map<std::string, AssetID> s_PathMap;
    };
} // namespace axiom