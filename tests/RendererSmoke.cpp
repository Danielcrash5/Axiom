#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/passes/ClearScreenPass.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

using namespace axiom::renderer;
using namespace axiom::renderer::rhi;

namespace {

// Protokoll der fuer die RenderGraph-Tests relevanten Befehle.
struct CmdEvent {
    enum class Kind { Transition, BeginRendering, ClearTexture } kind;
    TextureLayout from = TextureLayout::Undefined; // nur Transition
    TextureLayout to = TextureLayout::Undefined;   // nur Transition
    bool clear = false;                            // nur BeginRendering (loadOp)
};

class NullCommandList final : public CommandList {
public:
    explicit NullCommandList(std::vector<CmdEvent>* log = nullptr) : m_log(log) {}

    void copyBufferToBuffer(BufferHandle, uint64_t, BufferHandle, uint64_t,
                            uint64_t) override {}
    void transitionTexture(TextureHandle, TextureLayout from,
                           TextureLayout to) override {
        if (m_log)
            m_log->push_back({CmdEvent::Kind::Transition, from, to, false});
    }
    void clearTexture(TextureHandle, ClearColor) override {
        if (m_log)
            m_log->push_back({CmdEvent::Kind::ClearTexture});
    }
    void bindPipeline(PipelineHandle) override {}
    void bindVertexBuffer(uint32_t, BufferHandle) override {}
    void bindIndexBuffer(BufferHandle, bool) override {}
    void bindGroup(uint32_t, BindGroupHandle) override {}
    void draw(uint32_t, uint32_t, uint32_t, uint32_t) override {}
    void drawIndexed(uint32_t, uint32_t, uint32_t) override {}
    void dispatch(uint32_t, uint32_t, uint32_t) override {}
    void beginRendering(TextureHandle, std::optional<TextureHandle>,
                        std::optional<ClearColor> clearColor) override {
        if (m_log)
            m_log->push_back({CmdEvent::Kind::BeginRendering,
                              TextureLayout::Undefined, TextureLayout::Undefined,
                              clearColor.has_value()});
    }
    void endRendering() override {}

private:
    std::vector<CmdEvent>* m_log;
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
        return std::make_unique<NullCommandList>(&events);
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

    RHIResult<void> setSwapchainVsync(SwapchainHandle, bool) override {
        return {};
    }

    TextureFormat swapchainFormat(SwapchainHandle) const override {
        return TextureFormat::BGRA8Unorm;
    }

    void destroyBindGroupLayout(BindGroupLayoutHandle) override {
        ++destroyedBindGroupLayouts;
    }

    void destroyBindGroup(BindGroupHandle) override { ++destroyedBindGroups; }

    RHIResult<SamplerHandle> createSampler(const SamplerDesc&) override {
        return SamplerHandle{++m_nextSampler, 1};
    }

    void destroySampler(SamplerHandle) override { ++destroyedSamplers; }

    RHIResult<SwapchainHandle> createSwapchain(const SwapchainDesc&) override {
        return SwapchainHandle{++m_nextSwapchain, 1};
    }

    void destroySwapchain(SwapchainHandle) override { ++destroyedSwapchains; }

    RHIResult<AcquiredImage> acquireNextImage(SwapchainHandle) override {
        return AcquiredImage{TextureHandle{++m_nextTexture, 1}, 0};
    }

    RHIResult<void> present(SwapchainHandle, uint32_t) override {
        ++presents;
        return {};
    }

    std::pair<uint32_t, uint32_t> swapchainExtent(SwapchainHandle) const override {
        return {800, 600};
    }

    int createdTextures = 0;
    int destroyedTextures = 0;
    int destroyedBuffers = 0;
    int destroyedSurfaces = 0;
    int destroyedSwapchains = 0;
    int presents = 0;
    int destroyedPipelines = 0;
    int destroyedBindGroupLayouts = 0;
    int destroyedBindGroups = 0;
    int destroyedSamplers = 0;
    int submits = 0;
    std::vector<CmdEvent> events; // von allen CommandLists dieses Backends

private:
    uint32_t m_nextBuffer = 0;
    uint32_t m_nextTexture = 0;
    uint32_t m_nextSurface = 0;
    uint32_t m_nextPipeline = 0;
    uint32_t m_nextBindGroupLayout = 0;
    uint32_t m_nextBindGroup = 0;
    uint32_t m_nextSampler = 0;
    uint32_t m_nextSwapchain = 0;
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

TEST(RenderGraphTest, RecompileReusesUnchangedTransientTextures) {
    // Ersetzt den alten Test, der ein bedingungsloses Zerstoeren/Neuanlegen
    // bei JEDEM compile()-Aufruf erwartete - das war das Performance-Problem
    // aus dem Architektur-Review (Renderer::renderFrame() ruft compile() bei
    // jedem Frame auf, pro View). Jetzt: gleiche Resource-Beschreibung wird
    // wiederverwendet, nur eine tatsaechliche Aenderung (Resize) legt neu an.
    ::NullBackend backend;
    rendergraph::RenderGraph graph(backend);
    graph.addPass(std::make_unique<passes::ClearScreenPass>());

    ASSERT_TRUE(graph.compile());
    EXPECT_EQ(backend.createdTextures, 1);
    EXPECT_EQ(backend.destroyedTextures, 0);

    ASSERT_TRUE(graph.execute());
    EXPECT_EQ(backend.submits, 1);

    // Zweiter compile() OHNE View (Fallback 800x600, wie beim ersten Aufruf) -
    // Beschreibung ist unveraendert, muss wiederverwendet werden.
    ASSERT_TRUE(graph.compile());
    EXPECT_EQ(backend.createdTextures, 1);
    EXPECT_EQ(backend.destroyedTextures, 0);

    // Dritter compile() MIT View, die eine andere Viewport-Groesse hat -
    // ClearScreenPass deklariert jetzt eine andere Groesse, muss also
    // tatsaechlich neu angelegt werden.
    View resizedView;
    resizedView.viewport = Viewport{0, 0, 1920, 1080};
    rendergraph::RenderExecutionDesc resizedExecution{.view = &resizedView};

    ASSERT_TRUE(graph.compile(resizedExecution));
    EXPECT_EQ(backend.createdTextures, 2);
    EXPECT_EQ(backend.destroyedTextures, 1);

    graph.clear();
    EXPECT_EQ(backend.destroyedTextures, 2);
}

namespace {

// Schreibt das Present-Target mit einem festen Layout. Verhaelt sich wie die
// echten Passes: TransferDst -> clearTexture(), sonst beginRendering() mit
// loadOp CLEAR nur als erster Schreiber.
class PresentWritePass final : public rendergraph::RenderPass {
public:
    PresentWritePass(TextureLayout layout, std::vector<bool>* firstWrites,
                     bool legacyImport = false)
        : m_layout(layout), m_firstWrites(firstWrites),
          m_legacyImport(legacyImport) {}

    void setup(rendergraph::RenderGraphBuilder& builder) override {
        if (m_legacyImport) {
            // Altes Muster: Pass importiert das Swapchain-Image selbst.
            rendergraph::TextureResourceDesc desc{
                .width = 64,
                .height = 64,
                .format = TextureFormat::BGRA8Unorm,
                .usage = TextureUsage::RenderTarget,
                .debugName = "LegacyImport",
            };
            m_target = builder.write(
                builder.importTexture(builder.presentTarget(), desc), m_layout);
        } else {
            m_target = builder.write(builder.presentTargetResource(), m_layout);
        }
    }

    void execute(rendergraph::RenderContext& ctx, CommandList& cmd) override {
        const bool first = ctx.isFirstWrite(m_target);
        if (m_firstWrites)
            m_firstWrites->push_back(first);

        if (m_layout == TextureLayout::TransferDst) {
            cmd.clearTexture(ctx.resolveTexture(m_target), ClearColor{});
            return;
        }
        std::optional<ClearColor> clear;
        if (first)
            clear = ClearColor{};
        cmd.beginRendering(ctx.resolveTexture(m_target), std::nullopt, clear);
        cmd.endRendering();
    }

    const char* name() const override { return "PresentWritePass"; }

private:
    TextureLayout m_layout;
    std::vector<bool>* m_firstWrites;
    bool m_legacyImport;
    rendergraph::ResourceHandle m_target;
};

class PresentProbePass final : public rendergraph::RenderPass {
public:
    explicit PresentProbePass(bool* sawValidResource)
        : m_saw(sawValidResource) {}
    void setup(rendergraph::RenderGraphBuilder& builder) override {
        *m_saw = builder.presentTargetResource().valid();
    }
    void execute(rendergraph::RenderContext&, CommandList&) override {}
    const char* name() const override { return "PresentProbePass"; }

private:
    bool* m_saw;
};

rendergraph::RenderExecutionDesc makePresentExecution(const View& view) {
    return rendergraph::RenderExecutionDesc{
        .view = &view, .presentTarget = TextureHandle{7, 1}};
}

} // namespace

TEST(RenderGraphTest, TwoPassesShareOnePresentTargetAndSecondLoads) {
    // Zwei Passes schreiben das Swapchain-Image. Erwartet: EIN gemeinsamer
    // Resource-Entry, also nur EIN Uebergang Undefined -> ColorAttachment
    // (ein zweiter Uebergang von Undefined koennte den Inhalt des ersten
    // Passes verwerfen), der zweite Pass laedt statt zu clearen, und am
    // Ende genau ein Uebergang nach Present.
    ::NullBackend backend;
    rendergraph::RenderGraph graph(backend);
    std::vector<bool> firstWrites;
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::ColorAttachment, &firstWrites));
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::ColorAttachment, &firstWrites));

    View view;
    view.viewport = Viewport{0, 0, 640, 480};
    const auto execution = makePresentExecution(view);
    ASSERT_TRUE(graph.compile(execution));
    ASSERT_TRUE(graph.execute(execution));

    ASSERT_EQ(firstWrites.size(), 2u);
    EXPECT_TRUE(firstWrites[0]);
    EXPECT_FALSE(firstWrites[1]);

    using K = CmdEvent::Kind;
    ASSERT_EQ(backend.events.size(), 4u);
    EXPECT_EQ(backend.events[0].kind, K::Transition);
    EXPECT_EQ(backend.events[0].from, TextureLayout::Undefined);
    EXPECT_EQ(backend.events[0].to, TextureLayout::ColorAttachment);
    EXPECT_EQ(backend.events[1].kind, K::BeginRendering);
    EXPECT_TRUE(backend.events[1].clear);   // erster Schreiber cleart
    EXPECT_EQ(backend.events[2].kind, K::BeginRendering);
    EXPECT_FALSE(backend.events[2].clear);  // zweiter laedt
    EXPECT_EQ(backend.events[3].kind, K::Transition);
    EXPECT_EQ(backend.events[3].from, TextureLayout::ColorAttachment);
    EXPECT_EQ(backend.events[3].to, TextureLayout::Present);
}

TEST(RenderGraphTest, ImportsOfTheSameImageAreDeduplicated) {
    // Altes Muster: Passes importieren das Present-Target selbst (zusaetzlich
    // zum zentralen Import des Graphen). Muss trotzdem denselben Entry
    // ergeben, also dieselbe Ereignisfolge wie oben.
    ::NullBackend backend;
    rendergraph::RenderGraph graph(backend);
    std::vector<bool> firstWrites;
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::ColorAttachment, &firstWrites, /*legacyImport=*/true));
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::ColorAttachment, &firstWrites, /*legacyImport=*/true));

    View view;
    view.viewport = Viewport{0, 0, 640, 480};
    const auto execution = makePresentExecution(view);
    ASSERT_TRUE(graph.compile(execution));
    ASSERT_TRUE(graph.execute(execution));

    ASSERT_EQ(firstWrites.size(), 2u);
    EXPECT_TRUE(firstWrites[0]);
    EXPECT_FALSE(firstWrites[1]);
    ASSERT_EQ(backend.events.size(), 4u); // 1 Transition, 2 Begin, 1 Present
}

TEST(RenderGraphTest, LayoutFollowsTheAccessNotTheResource) {
    // Pass 1 cleart per Transfer, Pass 2 rendert als Attachment - auf
    // DERSELBEN Resource. Frueher haette die Resource-Usage beide auf ein
    // Layout gezwungen. Erwartet: Undefined -> TransferDst, dann
    // TransferDst -> ColorAttachment, Pass 2 laedt (Pass 1 hat schon
    // geschrieben), am Ende Present.
    ::NullBackend backend;
    rendergraph::RenderGraph graph(backend);
    std::vector<bool> firstWrites;
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::TransferDst, &firstWrites));
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::ColorAttachment, &firstWrites));

    View view;
    view.viewport = Viewport{0, 0, 640, 480};
    const auto execution = makePresentExecution(view);
    ASSERT_TRUE(graph.compile(execution));
    ASSERT_TRUE(graph.execute(execution));

    using K = CmdEvent::Kind;
    ASSERT_EQ(backend.events.size(), 5u);
    EXPECT_EQ(backend.events[0].kind, K::Transition);
    EXPECT_EQ(backend.events[0].to, TextureLayout::TransferDst);
    EXPECT_EQ(backend.events[1].kind, K::ClearTexture);
    EXPECT_EQ(backend.events[2].kind, K::Transition);
    EXPECT_EQ(backend.events[2].from, TextureLayout::TransferDst);
    EXPECT_EQ(backend.events[2].to, TextureLayout::ColorAttachment);
    EXPECT_EQ(backend.events[3].kind, K::BeginRendering);
    EXPECT_FALSE(backend.events[3].clear);
    EXPECT_EQ(backend.events[4].to, TextureLayout::Present);
}

TEST(RenderGraphTest, PresentTargetResourceIsInvalidWithoutSwapchainImage) {
    ::NullBackend backend;
    rendergraph::RenderGraph graph(backend);
    bool sawValid = true;
    graph.addPass(std::make_unique<PresentProbePass>(&sawValid));
    ASSERT_TRUE(graph.compile()); // keine View, kein Present-Target
    EXPECT_FALSE(sawValid);

    sawValid = false;
    View view;
    view.viewport = Viewport{0, 0, 640, 480};
    ASSERT_TRUE(graph.compile(makePresentExecution(view)));
    EXPECT_TRUE(sawValid);
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
