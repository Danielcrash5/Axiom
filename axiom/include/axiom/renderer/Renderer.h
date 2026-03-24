#pragma once

#include "axiom/renderer/VertexArray.h"

namespace axiom {

    class Renderer {
    public:

        static void Init();
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();

        static void Clear();

        static void DrawIndexed(
            const std::shared_ptr<VertexArray>& vao,
            uint32_t indexCount = 0,
            uint32_t indexOffset = 0
        );

    };

}