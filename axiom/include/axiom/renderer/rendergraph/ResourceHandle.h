/**
 * @file axiom/include/axiom/renderer/rendergraph/ResourceHandle.h
 * @brief Declares the public interface for the resource handle.
 */

#pragma once
#include "../rhi/Handle.h"

namespace axiom::renderer::rendergraph {

    // Eigener Handle-Typ, getrennt von rhi::TextureHandle/BufferHandle – das
    // sind Graph-interne "virtuelle" Resource-IDs, die erst beim Ausführen auf
    // echte GPU-Handles aufgelöst werden (Abschnitt 3 im Design-Doc).
    /** @brief Stores the data for the graph resource tag. */
    struct GraphResourceTag {};
    /**
     * @brief Provides the resource handle type alias.
     */
    using ResourceHandle = rhi::Handle<GraphResourceTag>;

} // namespace axiom::renderer::rendergraph