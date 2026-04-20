#include "axiom/renderer/Renderer.h"

#include <vector>

namespace axiom {

namespace {

bool MatricesEqual(const glm::mat4& a, const glm::mat4& b) {
    for (int column = 0; column < 4; ++column) {
        if (!glm::all(glm::equal(a[column], b[column])))
            return false;
    }

    return true;
}

} // namespace

void Renderer::Init() {
    RenderCommand::Init();
    s_IndirectBuffer = IndirectDrawBuffer::Create();
}

void Renderer::BeginScene(const glm::mat4& viewProjection, ClearState clearState) {
    if (clearState.ClearColor)
        RenderCommand::SetClearColor(clearState.Color);

    RenderCommand::SetClearState(clearState.ClearDepth, clearState.ClearColor);

    s_SceneData.ViewProjection = viewProjection;
    s_DrawQueue.clear();
    RenderCommand::Clear();
}

void Renderer::EndScene() {
    FlushQueue();
}

void Renderer::Flush() {
    FlushQueue();
}

const glm::mat4& Renderer::GetViewProjection() {
    return s_SceneData.ViewProjection;
}

void Renderer::Submit(const std::shared_ptr<Model>& model, const glm::mat4& transform) {
    if (!model)
        return;

    const auto& meshes = model->GetMeshes();
    const auto& materials = model->GetMaterials();

    for (const auto& mesh : meshes) {
        if (!mesh)
            continue;

        for (const auto& submesh : mesh->GetSubmeshes()) {
            if (submesh.MaterialIndex >= materials.size())
                continue;

            Submit(mesh->GetVertexArray(), materials[submesh.MaterialIndex], submesh.IndexCount, transform, submesh.IndexOffset);
        }
    }
}

void Renderer::Submit(
    const std::shared_ptr<VertexArray>& vao,
    const std::shared_ptr<Material>& material,
    uint32_t indexCount,
    const glm::mat4& transform
) {
    Submit(vao, material, indexCount, transform, 0);
}

void Renderer::SubmitLines(
    const std::shared_ptr<VertexArray>& vao,
    const std::shared_ptr<Material>& material,
    uint32_t indexCount
) {
    Submit(vao, material, indexCount, glm::mat4(1.0f), 0);
}

void Renderer::Submit(
    const std::shared_ptr<VertexArray>& vao,
    const std::shared_ptr<Material>& material,
    uint32_t indexCount,
    const glm::mat4& transform,
    uint32_t indexOffset
) {
    if (!vao || !material || indexCount == 0)
        return;

    s_DrawQueue.push_back({
        vao,
        material,
        indexCount,
        indexOffset,
        transform
    });
}

void Renderer::FlushQueue() {
    if (s_DrawQueue.empty())
        return;

    const bool useIndirect = s_IndirectBuffer && RenderCommand::SupportsIndirectRendering();
    size_t index = 0;

    while (index < s_DrawQueue.size()) {
        const DrawSubmission& first = s_DrawQueue[index];
        if (!first.VertexArray || !first.Material || first.IndexCount == 0) {
            ++index;
            continue;
        }

        size_t groupEnd = index + 1;
        while (groupEnd < s_DrawQueue.size()) {
            const DrawSubmission& next = s_DrawQueue[groupEnd];
            const bool compatible =
                next.VertexArray.get() == first.VertexArray.get() &&
                next.Material.get() == first.Material.get() &&
                MatricesEqual(next.Transform, first.Transform);

            if (!compatible)
                break;

            ++groupEnd;
        }

        first.Material->Set("u_ViewProjection", s_SceneData.ViewProjection);
        first.Material->Set("u_Transform", first.Transform);
        first.Material->Bind();

        if (useIndirect && groupEnd - index > 1) {
            std::vector<DrawElementsIndirectCommand> commands;
            commands.reserve(groupEnd - index);

            for (size_t commandIndex = index; commandIndex < groupEnd; ++commandIndex) {
                const DrawSubmission& submission = s_DrawQueue[commandIndex];
                commands.push_back({
                    submission.IndexCount,
                    1,
                    submission.IndexOffset,
                    0,
                    0
                });
            }

            s_IndirectBuffer->SetCommands(commands);
            RenderCommand::DrawIndexedIndirect(first.VertexArray, s_IndirectBuffer, static_cast<uint32_t>(commands.size()));
        }
        else {
            for (size_t commandIndex = index; commandIndex < groupEnd; ++commandIndex) {
                const DrawSubmission& submission = s_DrawQueue[commandIndex];
                RenderCommand::DrawIndexed(submission.VertexArray, submission.IndexCount, submission.IndexOffset);
            }
        }

        index = groupEnd;
    }

    s_DrawQueue.clear();
}

SceneData Renderer::s_SceneData;
std::vector<Renderer::DrawSubmission> Renderer::s_DrawQueue;
std::shared_ptr<IndirectDrawBuffer> Renderer::s_IndirectBuffer;

} // namespace axiom
