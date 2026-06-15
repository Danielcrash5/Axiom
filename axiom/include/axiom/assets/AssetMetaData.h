#pragma once

#include <string>

#include "AssetID.h"
#include "AssetType.h"

namespace axiom {
    struct AssetMetadata {
        AssetID ID {};

        AssetType Type = AssetType::Unknown;

        std::string Path;
        bool Loaded = false
    };
}