#include <axiom/renderer/passes/QuadRenderPass.h>

#include <axiom/core/Logger.h>
#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/ShaderRegistry.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <span>
#include <vector>

namespace axiom::renderer::passes {
namespace {

struct QuadVertex {
    float position[4];
    float color[4];
};

constexpr std::array<glm::vec3, 6> kUnitQuad{{
    {-0.5f, -0.5f, 0.0f},
    { 0.5f, -0.5f, 0.0f},
    { 0.5f,  0.5f, 0.0f},
    { 0.5f,  0.5f, 0.0f},
    {-0.5f,  0.5f, 0.0f},
    {-0.5f, -0.5f, 0.0f},
}};

} // namespace

QuadRenderPass::~QuadRenderPass() {
    if (!m_backend) {
        return;
    }
    if (m_pipeline.valid()) {
        m_backend->destroyPipeline(m_pipeline);
    }
    if (m_vertexBuffer.valid()) {
        m_backend->destroyBuffer(m_vertexBuffer);
    }
}

rhi::RHIResult<void> QuadRenderPass::ensureVertexCapacity(size_t vertexCount) {
    if (vertexCount <= m_vertexCapacity) {
        return {};
    }

    size_t newCapacity = std::max<size_t>(m_vertexCapacity, kUnitQuad.size());
    while (newCapacity < vertexCount) {
        if (newCapacity > std::numeric_limits<size_t>::max() / 2) {
            return std::unexpected(rhi::RHIError::OutOfMemory);
        }
        newCapacity *= 2;
    }
    if (newCapacity > std::numeric_limits<uint64_t>::max() /
                          sizeof(QuadVertex)) {
        return std::unexpected(rhi::RHIError::OutOfMemory);
    }

    auto buffer = m_backend->createBuffer({
        .sizeBytes = newCapacity * sizeof(QuadVertex),
        .usage = rhi::BufferUsage::Vertex | rhi::BufferUsage::CopyDst,
        .debugName = "SceneQuadVertices",
    });
    if (!buffer) {
        return std::unexpected(buffer.error());
    }

    const rhi::BufferHandle previousBuffer = m_vertexBuffer;
    m_vertexBuffer = *buffer;
    m_vertexCapacity = newCapacity;
    if (previousBuffer.valid()) {
        m_backend->destroyBuffer(previousBuffer);
    }
    return {};
}

rhi::RHIResult<void> QuadRenderPass::initialize(
    Renderer& renderer, rhi::SurfaceHandle surface,
    rhi::TextureFormat colorTargetFormat) {
    if (m_backend || !renderer.initialized() || !surface.valid()) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    m_backend = &renderer.backend();
    m_surface = surface;

    if (auto result = ensureVertexCapacity(kUnitQuad.size()); !result) {
        return std::unexpected(result.error());
    }

    rhi::VertexLayout vertexLayout{
        .stride = sizeof(QuadVertex),
        .attributes = {
            {.semantic = "POSITION", .location = 0,
             .format = rhi::VertexFormat::Float32x4,
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
            .blend = rhi::BlendMode::AlphaBlend,
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
    if (!m_target.valid() || !m_pipeline.valid() || !m_vertexBuffer.valid()) {
        return;
    }

    const auto items = ctx.items();
    std::vector<QuadVertex> vertices;
    vertices.reserve(items.size() * kUnitQuad.size());
    const View *view = ctx.currentView();
    const glm::mat4 viewProjection =
        view ? view->projectionMatrix * view->viewMatrix : glm::mat4(1.0f);

    for (const RenderItem &item : items) {
        const glm::mat4 transform = viewProjection * item.transform;
        std::array<QuadVertex, kUnitQuad.size()> quadVertices{};
        bool validQuad = true;
        for (size_t i = 0; i < kUnitQuad.size(); ++i) {
            const glm::vec4 clipPosition =
                transform * glm::vec4(kUnitQuad[i], 1.0f);
            if (std::abs(clipPosition.w) <= 1.0e-6f) {
                validQuad = false;
                break;
            }

            const glm::vec3 ndc =
                glm::vec3(clipPosition) / clipPosition.w;
            quadVertices[i] = {
                {ndc.x, ndc.y, ndc.z, 1.0f},
                {item.color.r, item.color.g, item.color.b, item.color.a},
            };
        }
        if (validQuad) {
            vertices.insert(vertices.end(), quadVertices.begin(),
                            quadVertices.end());
        }
    }

    if (vertices.size() > std::numeric_limits<uint32_t>::max()) {
        AXIOM_ERROR("QuadRenderPass: Zu viele Quad-Vertices fuer einen Draw-Aufruf");
        vertices.clear();
    }

    if (!vertices.empty()) {
        if (auto result = ensureVertexCapacity(vertices.size()); !result) {
            AXIOM_ERROR("QuadRenderPass: Vertex-Buffer konnte nicht erweitert werden");
            vertices.clear();
        } else if (auto result = m_backend->uploadBufferData(
                       m_vertexBuffer, 0, std::as_bytes(std::span{vertices}));
                   !result) {
            AXIOM_ERROR("QuadRenderPass: Scene-Vertices konnten nicht hochgeladen werden");
            vertices.clear();
        }
    }

    std::optional<rhi::ClearColor> clear;
    if (ctx.isFirstWrite(m_target)) {
        clear = rhi::ClearColor{0.05f, 0.05f, 0.08f, 1.0f};
    }

    cmd.beginRendering(ctx.resolveTexture(m_target), std::nullopt, clear);
    if (!vertices.empty()) {
        cmd.bindPipeline(m_pipeline);
        cmd.bindVertexBuffer(0, m_vertexBuffer);
        cmd.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }
    cmd.endRendering();
}

} // namespace axiom::renderer::passes
