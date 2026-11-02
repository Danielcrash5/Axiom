#include <axiom/renderer/passes/QuadRenderPass.h>

#include <axiom/core/Logger.h>
#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/ShaderRegistry.h>

#include <array>
#include <cstddef>
#include <span>

namespace axiom::renderer::passes {
namespace {

struct QuadVertex {
    float position[2];
    float color[4];
};

constexpr std::array<QuadVertex, 4> kVertices{{
    {{-0.65f, -0.45f}, {0.10f, 0.75f, 0.95f, 1.0f}},
    {{ 0.65f, -0.45f}, {0.10f, 0.75f, 0.95f, 1.0f}},
    {{ 0.65f,  0.45f}, {0.10f, 0.75f, 0.95f, 1.0f}},
    {{-0.65f,  0.45f}, {0.10f, 0.75f, 0.95f, 1.0f}},
}};

constexpr std::array<uint16_t, 6> kIndices{{0, 1, 2, 2, 3, 0}};

template <typename T>
std::span<const std::byte> AsBytes(const T& value) {
    return std::as_bytes(std::span{&value, size_t{1}});
}

} // namespace

QuadRenderPass::~QuadRenderPass() {
    if (!m_backend) {
        return;
    }
    if (m_pipeline.valid()) {
        m_backend->destroyPipeline(m_pipeline);
    }
    if (m_indexBuffer.valid()) {
        m_backend->destroyBuffer(m_indexBuffer);
    }
    if (m_vertexBuffer.valid()) {
        m_backend->destroyBuffer(m_vertexBuffer);
    }
}

rhi::RHIResult<void> QuadRenderPass::initialize(
    Renderer& renderer, rhi::SurfaceHandle surface,
    rhi::TextureFormat colorTargetFormat) {
    if (m_backend || !renderer.initialized() || !surface.valid()) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    m_backend = &renderer.backend();
    m_surface = surface;

    auto vertexBuffer = m_backend->createBuffer({
        .sizeBytes = sizeof(kVertices),
        .usage = rhi::BufferUsage::Vertex | rhi::BufferUsage::CopyDst,
        .debugName = "VisibleQuadVertices",
    });
    if (!vertexBuffer) {
        return std::unexpected(vertexBuffer.error());
    }
    m_vertexBuffer = *vertexBuffer;

    if (auto result = m_backend->uploadBufferData(
            m_vertexBuffer, 0, AsBytes(kVertices)); !result) {
        return std::unexpected(result.error());
    }

    auto indexBuffer = m_backend->createBuffer({
        .sizeBytes = sizeof(kIndices),
        .usage = rhi::BufferUsage::Index | rhi::BufferUsage::CopyDst,
        .debugName = "VisibleQuadIndices",
    });
    if (!indexBuffer) {
        return std::unexpected(indexBuffer.error());
    }
    m_indexBuffer = *indexBuffer;

    if (auto result = m_backend->uploadBufferData(
            m_indexBuffer, 0, AsBytes(kIndices)); !result) {
        return std::unexpected(result.error());
    }

    rhi::VertexLayout vertexLayout{
        .stride = sizeof(QuadVertex),
        .attributes = {
            {.semantic = "POSITION", .location = 0,
             .format = rhi::VertexFormat::Float32x2,
             .offset = offsetof(QuadVertex, position)},
            {.semantic = "COLOR", .location = 1,
             .format = rhi::VertexFormat::Float32x4,
             .offset = offsetof(QuadVertex, color)},
        },
    };

    auto shaderId = renderer.shaderRegistry().compileFromSource(
        "engine://shaders/Renderer2D/Quad.vert.glsl",
        "engine://shaders/Renderer2D/Quad.frag.glsl",
        std::move(vertexLayout));
    if (!shaderId) {
        return std::unexpected(shaderId.error());
    }

    const rhi::ShaderDesc* shader = renderer.shaderRegistry().find(*shaderId);
    if (!shader) {
        AXIOM_ERROR("QuadRenderPass: kompilierter Shader fehlt im ShaderRegistry");
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    rhi::PipelineDesc pipelineDesc{
        .shader = *shader,
        .state = {
            .blend = rhi::BlendMode::None,
            .depthTest = false,
            .depthWrite = false,
            .cull = rhi::CullMode::None,
            .topology = rhi::PrimitiveTopology::TriangleList,
        },
        .colorTargetFormat = colorTargetFormat,
    };
    auto pipeline = m_backend->createPipeline(pipelineDesc);
    if (!pipeline) {
        return std::unexpected(pipeline.error());
    }
    m_pipeline = *pipeline;
    return {};
}

void QuadRenderPass::setup(rendergraph::RenderGraphBuilder& builder) {
    m_target = {};
    const View* view = builder.currentView();
    const auto presentTarget = builder.presentTargetResource();
    if (!view || !presentTarget.valid() ||
        view->target.kind != RenderTargetKind::Swapchain ||
        !(view->target.surface == m_surface)) {
        return;
    }

    m_target = builder.write(presentTarget, rhi::TextureLayout::ColorAttachment);
}

void QuadRenderPass::execute(rendergraph::RenderContext& ctx,
                             rhi::CommandList& cmd) {
    if (!m_target.valid() || !m_pipeline.valid() ||
        !m_vertexBuffer.valid() || !m_indexBuffer.valid()) {
        return;
    }

    std::optional<rhi::ClearColor> clear;
    if (ctx.isFirstWrite(m_target)) {
        clear = rhi::ClearColor{0.05f, 0.05f, 0.08f, 1.0f};
    }

    cmd.beginRendering(ctx.resolveTexture(m_target), std::nullopt, clear);
    cmd.bindPipeline(m_pipeline);
    cmd.bindVertexBuffer(0, m_vertexBuffer);
    cmd.bindIndexBuffer(m_indexBuffer, true);
    cmd.drawIndexed(static_cast<uint32_t>(kIndices.size()), 1, 0);
    cmd.endRendering();
}

} // namespace axiom::renderer::passes
