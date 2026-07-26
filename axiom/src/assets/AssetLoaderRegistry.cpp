#include <axiom/assets/AssetLoaderRegistry.h>

namespace axiom {

    void AssetLoaderRegistry::RegisterLoader(AssetType type, AssetLoaderFn loader) {
        s_Loaders[type] = std::move(loader);
    }

    const AssetLoaderFn *AssetLoaderRegistry::Find(AssetType type) {
        auto it = s_Loaders.find(type);
        if (it == s_Loaders.end()) return nullptr;
        return &it->second;
    }

} // namespace axiom
