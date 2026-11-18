/**
 * @file axiom/include/axiom/assets/AssetMetaData.h
 * @brief Declares the public interface for the asset meta data.
 */

#pragma once

#include <string>

#include "TypedUUID.h"
#include "AssetType.h"

namespace axiom {
    /** @brief Stores the data for the asset metadata. */
    struct AssetMetadata {
        /** @brief Stores the ID. */
        TypedUUID ID{};

        /** @brief Stores the type. */
        AssetType Type = AssetType::Unknown;

        /** @brief Stores the path. */
        std::string Path;
        /** @brief Stores the loaded. */
        bool Loaded = false;
    };
} // namespace axiom
