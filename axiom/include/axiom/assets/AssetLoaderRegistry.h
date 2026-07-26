#pragma once
#include <functional>
#include <memory>
#include <unordered_map>

#include "Asset.h"
#include "AssetMetadata.h"
#include "AssetType.h"

namespace axiom {

    // Ladefunktion pro AssetType, statt hartcodiertem Switch in AssetLoader -
    // neue Asset-Typen (auch von ausserhalb des Core-Engine-Codes, z.B.
    // axiom_renderer fuer Shader-Assets) registrieren sich hier selbst,
    // statt eine zentrale Datei anfassen zu muessen.
    using AssetLoaderFn = std::function<std::shared_ptr<Asset>(const AssetMetadata &)>;

    class AssetLoaderRegistry {
      public:
        static void RegisterLoader(AssetType type, AssetLoaderFn loader);
        static const AssetLoaderFn *Find(AssetType type);

      private:
        static inline std::unordered_map<AssetType, AssetLoaderFn> s_Loaders;
    };

} // namespace axiom
