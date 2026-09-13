#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/passes/ClearScreenPass.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

using namespace axiom::renderer;
using namespace axiom::renderer::rhi;

namespace {

class NullCommandList final : public CommandList {
public:
    void copyBufferToBuffer(BufferHandle, uint64_t, BufferHandle, uint64_t,
                            uint64_t) override {}
    void transitionTexture(TextureHandle, TextureLayout, TextureLayout) override {}
    void clearTexture(TextureHandle, ClearColor) override {}
    void bindPipeline(PipelineHandle) override {}
    void bindVertexBuffer(uint32_t, BufferHandle) override {}
    void bindIndexBuffer(BufferHandle, bool) override {}
    void bindGroup(uint32_t, BindGroupHandle) override {}
    void draw(uint32_t, uint32_t, uint32_t, uint32_t) override {}
    void drawIndexed(uint32_t, uint32_t, uint32_t) override {}
    void dispatch(uint32_t, uint32_t, uint32_t) override {}
    void beginRendering(TextureHandle, std::optional<TextureHandle>) override {}
    void endRendering() override {}
};

class NullBackend final : public IRHIBackend {
public:
    RHIResult<BufferHandle> createBuffer(const BufferDesc&) override {
        return BufferHandle{++m_nextBuffer, 1};
    }

    RHIResult<TextureHandle> createTexture(const TextureDesc&) override {
        ++createdTextures;
        return TextureHandle{++m_nextTexture, 1};
    }

    void destroyBuffer(BufferHandle) override { ++destroyedBuffers; }
    void destroyTexture(TextureHandle) override { ++destroyedTextures; }

    RHIResult<void> uploadBufferData(BufferHandle, uint64_t,
                                     std::span<const std::byte>) override {
        return {};
    }

    RHIResult<void> uploadTextureData(TextureHandle, const TextureUploadDesc&,
                                      std::span<const std::byte>) override {
        return {};
    }

    std::unique_ptr<CommandList> createCommandList() override {
        return std::make_unique<NullCommandList>();
    }

    void submit(CommandList&) override { ++submits; }

    void* nativeInstanceHandle() const override {
        return reinterpret_cast<void*>(0x1);
    }

    RHIResult<SurfaceHandle> createSurface(void*) override {
        return SurfaceHandle{++m_nextSurface, 1};
    }

    void destroySurface(SurfaceHandle) override { ++destroyedSurfaces; }

    RHIResult<PipelineHandle> createPipeline(const PipelineDesc&) override {
        return PipelineHandle{++m_nextPipeline, 1};
    }

    void destroyPipeline(PipelineHandle) override { ++destroyedPipelines; }

    RHIResult<BindGroupLayoutHandle>
    createBindGroupLayout(const BindGroupLayoutDesc&) override {
        return BindGroupLayoutHandle{++m_nextBindGroupLayout, 1};
    }

    RHIResult<BindGroupHandle> createBindGroup(const BindGroupDesc&) override {
        return BindGroupHandle{++m_nextBindGroup, 1};
    }

    RHIResult<void> updateBindGroup(BindGroupHandle,
                                    std::span<const BindGroupEntry>) override {
        return {};
    }

    void destroyBindGroupLayout(BindGroupLayoutHandle) override {
        ++destroyedBindGroupLayouts;
    }

    void destroyBindGroup(BindGroupHandle) override { ++destroyedBindGroups; }

    RHIResult<SamplerHandle> createSampler(const SamplerDesc&) override {
        return SamplerHandle{++m_nextSampler, 1};
    }

    void destroySampler(SamplerHandle) override { ++destroyedSamplers; }

    int createdTextures = 0;
    int destroyedTextures = 0;
    int destroyedBuffers = 0;
    int destroyedSurfaces = 0;
    int destroyedPipelines = 0;
    int destroyedBindGroupLayouts = 0;
    int destroyedBindGroups = 0;
    int destroyedSamplers = 0;
    int submits = 0;

private:
    uint32_t m_nextBuffer = 0;
    uint32_t m_nextTexture = 0;
    uint32_t m_nextSurface = 0;
    uint32_t m_nextPipeline = 0;
    uint32_t m_nextBindGroupLayout = 0;
    uint32_t m_nextBindGroup = 0;
    uint32_t m_nextSampler = 0;
};

struct ProbeFrame {
    int priority = 0;
    uint32_t viewportWidth = 0;
    std::vector<int32_t> zOrder;
};

class QueueProbePass final : public rendergraph::RenderPass {
public:
    QueueProbePass(std::vector<ProbeFrame>& frames,
                   RenderQueueDescriptor descriptor)
        : m_frames(frames), m_descriptor(descriptor) {}

    void setup(rendergraph::RenderGraphBuilder&) override {}

    void execute(rendergraph::RenderContext& ctx, CommandList&) override {
        ProbeFrame frame;
        if (const View* view = ctx.currentView()) {
            frame.priority = view->priority;
            frame.viewportWidth = view->viewport.width;
        }

        for (const RenderItem& item : ctx.items()) {
            frame.zOrder.push_back(item.zIndex);
        }
        m_frames.push_back(std::move(frame));
    }

    const char* name() const override { return "QueueProbePass"; }
    bool usesRenderQueue() const override { return true; }
    RenderQueueDescriptor queueDescriptor() const override {
        return m_descriptor;
    }

private:
    std::vector<ProbeFrame>& m_frames;
    RenderQueueDescriptor m_descriptor;
};

RenderItem makeItem(RenderLayerMask layer, int32_t zIndex, float depth,
                    std::shared_ptr<Material> material) {
    RenderItem item;
    item.layer = layer;
    item.zIndex = zIndex;
    item.depth = depth;
    item.material = std::move(material);
    item.vertexBuffer = BufferHandle{1, 1};
    item.indexBuffer = BufferHandle{2, 1};
    item.indexCount = 6;
    return item;
}

} // namespace

TEST(RenderQueueTest, FiltersSortsAndBatchesItems) {
    auto material = std::make_shared<Material>();
    material->shaderId = static_cast<ShaderID>(7);

    std::vector<RenderItem> items;
    items.push_back(makeItem(0b001, 20, 1.0f, material));
    items.push_back(makeItem(0b001, 10, 5.0f, material));
    items.push_back(makeItem(0b010, 30, 3.0f, material));

    RenderQueueSystem queues;
    RenderQueueDescriptor descriptor{
        .acceptedLayers = 0b001,
        .sortMode = SortMode::BackToFront,
    };

    auto result = queues.build(items, std::span<const RenderQueueDescriptor>(&descriptor, 1),
                               0b001);

    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0].size(), 2u);
    EXPECT_EQ(result[0][0].zIndex, 10);
    EXPECT_EQ(result[0][1].zIndex, 20);

    auto batches = BatchBuilder::build(result[0]);
    ASSERT_EQ(batches.size(), 1u);
    EXPECT_EQ(batches[0].instanceTransforms.size(), 2u);
}

TEST(RenderGraphTest, RecompileReleasesPreviousTransientTextures) {
    NullBackend backend;
    rendergraph::RenderGraph graph(backend);
    graph.addPass(std::make_unique<passes::ClearScreenPass>());

    ASSERT_TRUE(graph.compile());
    EXPECT_EQ(backend.createdTextures, 1);
    EXPECT_EQ(backend.destroyedTextures, 0);

    ASSERT_TRUE(graph.execute());
    EXPECT_EQ(backend.submits, 1);

    ASSERT_TRUE(graph.compile());
    EXPECT_EQ(backend.createdTextures, 2);
    EXPECT_EQ(backend.destroyedTextures, 1);

    graph.clear();
    EXPECT_EQ(backend.destroyedTextures, 2);
}

TEST(RendererTest, RenderFrameRunsViewsInPriorityOrderAndQueuesVisibleItems) {
    auto backend = std::make_unique<NullBackend>();
    NullBackend* backendPtr = backend.get();

    Renderer renderer;
    RendererServicesDesc services;
    services.initializeAssetManager = false;
    services.initializeBindlessTextureHeap = false;
    ASSERT_TRUE(renderer.init(std::move(backend), services));

    std::vector<ProbeFrame> frames;
    renderer.addPass(std::make_unique<QueueProbePass>(
        frames, RenderQueueDescriptor{.acceptedLayers = 0b011,
                                      .sortMode = SortMode::BackToFront}));

    View lateView;
    lateView.visibleLayers = 0b001;
    lateView.viewport.width = 111;
    lateView.priority = 10;
    const ViewID lateViewId = renderer.registerView(lateView);

    View earlyView;
    earlyView.visibleLayers = 0b010;
    earlyView.viewport.width = 222;
    earlyView.priority = -5;
    const ViewID earlyViewId = renderer.registerView(earlyView);
    EXPECT_NE(lateViewId, earlyViewId);

    auto material = std::make_shared<Material>();
    material->shaderId = static_cast<ShaderID>(3);

    renderer.submitItem(makeItem(0b001, 1, 1.0f, material));
    renderer.submitItem(makeItem(0b010, 2, 2.0f, material));
    renderer.submitItem(makeItem(0b100, 3, 3.0f, material));

    ASSERT_TRUE(renderer.renderFrame());

    ASSERT_EQ(frames.size(), 2u);
    EXPECT_EQ(frames[0].priority, -5);
    EXPECT_EQ(frames[0].viewportWidth, 222u);
    ASSERT_EQ(frames[0].zOrder.size(), 1u);
    EXPECT_EQ(frames[0].zOrder[0], 2);

    EXPECT_EQ(frames[1].priority, 10);
    EXPECT_EQ(frames[1].viewportWidth, 111u);
    ASSERT_EQ(frames[1].zOrder.size(), 1u);
    EXPECT_EQ(frames[1].zOrder[0], 1);

    EXPECT_TRUE(renderer.itemPool().empty());
    EXPECT_EQ(backendPtr->submits, 2);
}
