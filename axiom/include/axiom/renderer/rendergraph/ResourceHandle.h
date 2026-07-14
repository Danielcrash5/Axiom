#pragma once
#include "../rhi/Handle.h"

namespace axiom::renderer::rendergraph {

    // Eigener Handle-Typ, getrennt von rhi::TextureHandle/BufferHandle – das
    // sind Graph-interne "virtuelle" Resource-IDs, die erst beim Ausführen auf
    // echte GPU-Handles aufgelöst werden (Abschnitt 3 im Design-Doc).
    struct GraphResourceTag {};
    using ResourceHandle = rhi::Handle<GraphResourceTag>;

} // namespace axiom::renderer::rendergraph