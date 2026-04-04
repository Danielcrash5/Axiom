#include "axiom/assetmanager/AssetType.h"

namespace axiom {

    const char* AssetTypeToString(AssetType type) {
        switch (type) {
        case AssetType::Texture2D: return "Texture2D";
        case AssetType::Shader:    return "Shader";
        case AssetType::Audio:     return "Audio";
        default:                   return "None";
        }
    }

    AssetType AssetTypeFromString(const std::string& str) {
        if (str == "Texture2D") return AssetType::Texture2D;
        if (str == "Shader")    return AssetType::Shader;
        if (str == "Audio")     return AssetType::Audio;

        return AssetType::None;
    }

}