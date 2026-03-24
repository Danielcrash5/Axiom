#include "axiom/renderer/Renderer.h"
#include <glad/glad.h>

namespace axiom {

    void Renderer::Init() {
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void Renderer::Shutdown() {
    }

    void Renderer::BeginFrame() {
    }

    void Renderer::EndFrame() {
    }

    void Renderer::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::DrawIndexed(
        const std::shared_ptr<VertexArray>& vao,
        uint32_t indexCount,
        uint32_t indexOffset) {
        vao->Bind();

        uint32_t count = indexCount;

        if (count == 0)
            count = vao->GetIndexBuffer()->GetCount();

        glDrawElements(
            GL_TRIANGLES,
            count,
            GL_UNSIGNED_INT,
            (const void*)(uintptr_t)(indexOffset * sizeof(uint32_t))
        );

}