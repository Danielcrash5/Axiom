#pragma once

namespace axiom {
enum class AssetType {
    Unknown = 0,

    Texture,
    Shader,
    Material,

    Mesh,
    Scene,

    Audio,

    Script
};
}