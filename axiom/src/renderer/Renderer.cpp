#include "axiom/renderer/Renderer.h"
#include <glad/glad.h>

namespace axiom {

    RenderState Renderer::s_CurrentState {};

    void Renderer::Init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void Renderer::Shutdown() {
    }

    void Renderer::Begin(const glm::vec4& clearColor) {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::End() {
    }

    void Renderer::SetRenderState(const RenderState& state) {
        // Depth Test
        if (state.DepthTest != s_CurrentState.DepthTest) {
            state.DepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        }

        // Depth Write
        if (state.DepthWrite != s_CurrentState.DepthWrite) {
            glDepthMask(state.DepthWrite ? GL_TRUE : GL_FALSE);
        }

        // Depth Func
        if (state.DepthFunc != s_CurrentState.DepthFunc) {
            glDepthFunc(state.DepthFunc);
        }

        // Blending
        if (state.Blend != s_CurrentState.Blend) {
            state.Blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
        }

        if (state.BlendSrc != s_CurrentState.BlendSrc ||
            state.BlendDst != s_CurrentState.BlendDst) {
            glBlendFunc(state.BlendSrc, state.BlendDst);
        }

        // Cull Face
        if (state.CullFace != s_CurrentState.CullFace) {
            state.CullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
        }

        if (state.CullMode != s_CurrentState.CullMode) {
            glCullFace(state.CullMode);
        }

        // Stencil (minimal, später erweitern)
        if (state.StencilTest != s_CurrentState.StencilTest) {
            state.StencilTest ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
        }

        s_CurrentState = state;
    }

    void Renderer::DrawIndexed(const VertexArray& vao, uint32_t count) {
        vao.bind();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }

}