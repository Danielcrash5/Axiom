#pragma once
#include <string>

namespace axiom {

    enum class AssetType : uint16_t {
        None = 0,

        Texture2D,
        Shader,
        Audio,

        // später:
        // Material,
        // Mesh,
        // Model,
        // Scene
    };

    const char* AssetTypeToString(AssetType type);
    AssetType AssetTypeFromString(const std::string& str);

}