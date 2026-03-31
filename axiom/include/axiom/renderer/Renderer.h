#pragma once
#include "RenderState.h"
#include "VertexArray.h"
#include <glm/glm.hpp>

namespace axiom {

    class Renderer {
    public:
        static void Init();
        static void Shutdown();

        static void Begin(const glm::vec4& clearColor = { 0,0,0,1 });
        static void End();

        static void SetRenderState(const RenderState& state);

        static void DrawIndexed(const VertexArray& vao, uint32_t indexCount = 0);

    private:
        static RenderState s_CurrentState;
    };

}