#pragma once

#include <string>

#include "TypedUUID.h"
#include "AssetType.h"

namespace axiom {
    struct AssetMetadata {
        TypedUUID ID{};

        AssetType Type = AssetType::Unknown;

        std::string Path;
        bool Loaded = false;
    };
} // namespace axiom
