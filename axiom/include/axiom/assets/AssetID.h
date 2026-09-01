#pragma once

#include "TypedUUID.h"

namespace axiom {
    // AssetUUID is the primary asset identifier type (based on TypedUUID)
    // It's a 128-bit UUID with embedded AssetTypeId
    using AssetUUID = TypedUUID;
    
    // Note: AssetID is defined in axiom/core/TypedUUID.h for ECS use
    // Asset code should prefer AssetUUID for asset identification
} // namespace axiom