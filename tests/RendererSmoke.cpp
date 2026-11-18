/**
 * @file tests/RendererSmoke.cpp
 * @brief Implements the renderer smoke module.
 */

#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/Renderer2D.h>
#include <axiom/renderer/passes/ClearScreenPass.h>
#include <axiom/renderer/passes/QuadRenderPass.h>
#include <axiom/assets/VFS.h>
#include <axiom/ecs/Entity.h>
#include <axiom/ecs/Render2DSystem.h>
#include <axiom/ecs/Scene.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

using namespace axiom::renderer;
using namespace axiom::renderer::rhi;

namespace {

// Protokoll der fuer die RenderGraph-Tests relevanten Befehle.
/** @brief Carries the data for the cmd event event. */
struct CmdEvent {
    /**
     * @brief Defines the available kind values.
     * @tparam Kind Template type used by this type.
     */
    enum class Kind { transition, beginRendering, clearTexture } kind;
    /** @brief Stores the from. */
    TextureLayout from = TextureLayout::undefined; // nur Transition
    /** @brief Stores the to. */
    TextureLayout to = TextureLayout::undefined;   // nur Transition
    /** @brief Stores the clear. */
    bool clear = false;                            // nur BeginRendering (loadOp)
};

/**
 * @brief Provides the null command list functionality.
 * @tparam NullCommandList Template type used by this type.
 */
class NullCommandList final : public CommandList {
public:
    /**
     * @brief Creates an instance of null command list.
     * @param log The log used by this operation.
     * @param indexedDrawCalls The indexed draw calls used by this operation.
     * @param drawVertexCounts The draw vertex counts used by this operation.
     */
    explicit NullCommandList(std::vector<CmdEvent>* log = nullptr,
                             int* indexedDrawCalls = nullptr,
                             std::vector<uint32_t>* drawVertexCounts = nullptr)
        : m_log(log), m_indexedDrawCalls(indexedDrawCalls),
          m_drawVertexCounts(drawVertexCounts) {}

    /**
     * @brief Copies a range of bytes between buffers.
     */
    void copyBufferToBuffer(BufferHandle, uint64_t, BufferHandle, uint64_t,
                            uint64_t) override {}
    /**
     * @brief Transitions a texture to the requested layout.
     * @param from The from used by this operation.
     * @param to The to used by this operation.
     */
    void transitionTexture(TextureHandle, TextureLayout from,
                           TextureLayout to) override {
        if (m_log)
            m_log->push_back({CmdEvent::Kind::transition, from, to, false});
    }
    /**
     * @brief Clears texture.
     */
    void clearTexture(TextureHandle, ClearColor) override {
        if (m_log)
            m_log->push_back({CmdEvent::Kind::clearTexture});
    }
    /**
     * @brief Binds pipeline.
     */
    void bindPipeline(PipelineHandle) override {}
    /**
     * @brief Binds vertex buffer.
     */
    void bindVertexBuffer(uint32_t, BufferHandle) override {}
    /**
     * @brief Binds index buffer.
     */
    void bindIndexBuffer(BufferHandle, bool) override {}
    /**
     * @brief Binds group.
     */
    void bindGroup(uint32_t, BindGroupHandle) override {}
    /**
     * @brief Draws the requested geometry.
     * @param vertexCount The vertex count used by this operation.
     */
    void draw(uint32_t vertexCount, uint32_t, uint32_t, uint32_t) override {
        if (m_drawVertexCounts) {
            m_drawVertexCounts->push_back(vertexCount);
        }
    }
    /**
     * @brief Draws indexed.
     */
    void drawIndexed(uint32_t, uint32_t, uint32_t) override {
        if (m_indexedDrawCalls) {
            ++*m_indexedDrawCalls;
        }
    }
    /**
     * @brief Dispatches the pending event or command.
     */
    void dispatch(uint32_t, uint32_t, uint32_t) override {}
    /**
     * @brief Begins rendering.
     * @param TextureHandle The texture handle used by this operation.
     * @param clearColor The clear color used by this operation.
     */
    void beginRendering(TextureHandle, std::optional<TextureHandle>,
                        std::optional<ClearColor> clearColor) override {
        if (m_log)
            m_log->push_back({CmdEvent::Kind::beginRendering,
                              TextureLayout::undefined, TextureLayout::undefined,
                              clearColor.has_value()});
    }
    /**
     * @brief Ends rendering.
     */
    void endRendering() override {}

private:
    /** @brief Stores the log. */
    std::vector<CmdEvent>* m_log;
    /** @brief Stores the indexed draw calls. */
    int* m_indexedDrawCalls;
    /** @brief Stores the draw vertex counts. */
    std::vector<uint32_t>* m_drawVertexCounts;
};

/**
 * @brief Provides the null backend functionality.
 * @tparam NullBackend Template type used by this type.
 */
class NullBackend final : public IRHIBackend {
public:
    /**
     * @brief Creates buffer.
     * @param BufferDesc The buffer desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<BufferHandle> createBuffer(const BufferDesc&) override {
        return BufferHandle{++m_nextBuffer, 1};
    }

    /**
     * @brief Creates texture.
     * @param TextureDesc The texture desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<TextureHandle> createTexture(const TextureDesc&) override {
        ++createdTextures;
        return TextureHandle{++m_nextTexture, 1};
    }

    /**
     * @brief Destroys the buffer and releases its resources.
     */
    void destroyBuffer(BufferHandle) override { ++destroyedBuffers; }
    /**
     * @brief Destroys the texture and releases its resources.
     */
    void destroyTexture(TextureHandle) override { ++destroyedTextures; }

    /**
     * @brief Uploads data to a buffer.
     * @param byte The byte used by this operation.
     */
    RHIResult<void> uploadBufferData(BufferHandle, uint64_t,
                                     std::span<const std::byte>) override {
        return {};
    }

    /**
     * @brief Uploads data to a texture.
     * @param TextureUploadDesc The texture upload desc used by this operation.
     * @param byte The byte used by this operation.
     */
    RHIResult<void> uploadTextureData(TextureHandle, const TextureUploadDesc&,
                                      std::span<const std::byte>) override {
        return {};
    }

    /**
     * @brief Creates command list.
     * @return The result of the operation.
     */
    std::unique_ptr<CommandList> createCommandList() override {
        return std::make_unique<NullCommandList>(&events, &indexedDrawCalls,
                                                 &drawVertexCounts);
    }

    /**
     * @brief Submits the collected work for execution.
     */
    void submit(CommandList&) override { ++submits; }

    /**
     * @brief Returns the native instance handle.
     * @return The result of the operation.
     */
    void* nativeInstanceHandle() const override {
        return reinterpret_cast<void*>(0x1);
    }

    /**
     * @brief Creates surface.
     * @return The result of the operation.
     */
    RHIResult<SurfaceHandle> createSurface(void*) override {
        return SurfaceHandle{++m_nextSurface, 1};
    }

    /**
     * @brief Destroys the surface and releases its resources.
     */
    void destroySurface(SurfaceHandle) override { ++destroyedSurfaces; }

    /**
     * @brief Creates pipeline.
     * @param PipelineDesc The pipeline desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<PipelineHandle> createPipeline(const PipelineDesc&) override {
        return PipelineHandle{++m_nextPipeline, 1};
    }

    /**
     * @brief Destroys the pipeline and releases its resources.
     */
    void destroyPipeline(PipelineHandle) override { ++destroyedPipelines; }

    /**
     * @brief Creates bind group layout.
     * @param BindGroupLayoutDesc The bind group layout desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<BindGroupLayoutHandle>
    createBindGroupLayout(const BindGroupLayoutDesc&) override {
        return BindGroupLayoutHandle{++m_nextBindGroupLayout, 1};
    }

    /**
     * @brief Creates bind group.
     * @param BindGroupDesc The bind group desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<BindGroupHandle> createBindGroup(const BindGroupDesc&) override {
        return BindGroupHandle{++m_nextBindGroup, 1};
    }

    /**
     * @brief Updates bind group.
     * @param BindGroupEntry The bind group entry used by this operation.
     */
    RHIResult<void> updateBindGroup(BindGroupHandle,
                                    std::span<const BindGroupEntry>) override {
        return {};
    }

    /**
     * @brief Sets swapchain vsync.
     */
    RHIResult<void> setSwapchainVsync(SwapchainHandle, bool) override {
        return {};
    }

    /**
     * @brief Returns the swapchain format.
     * @return The result of the operation.
     */
    TextureFormat swapchainFormat(SwapchainHandle) const override {
        return TextureFormat::bgrA8Unorm;
    }

    /**
     * @brief Destroys the bind group layout.
     */
    void destroyBindGroupLayout(BindGroupLayoutHandle) override {
        ++destroyedBindGroupLayouts;
    }

    /**
     * @brief Destroys the bind group.
     */
    void destroyBindGroup(BindGroupHandle) override { ++destroyedBindGroups; }

    /**
     * @brief Creates sampler.
     * @param SamplerDesc The sampler desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<SamplerHandle> createSampler(const SamplerDesc&) override {
        return SamplerHandle{++m_nextSampler, 1};
    }

    /**
     * @brief Destroys the sampler.
     */
    void destroySampler(SamplerHandle) override { ++destroyedSamplers; }

    /**
     * @brief Creates swapchain.
     * @param SwapchainDesc The swapchain desc used by this operation.
     * @return The result of the operation.
     */
    RHIResult<SwapchainHandle> createSwapchain(const SwapchainDesc&) override {
        return SwapchainHandle{++m_nextSwapchain, 1};
    }

    /**
     * @brief Destroys the swapchain and releases its resources.
     */
    void destroySwapchain(SwapchainHandle) override { ++destroyedSwapchains; }

    /**
     * @brief Acquires the next image from the swapchain.
     * @return The result of the operation.
     */
    RHIResult<AcquiredImage> acquireNextImage(SwapchainHandle) override {
        return AcquiredImage{TextureHandle{++m_nextTexture, 1}, 0};
    }

    /**
     * @brief Presents the rendered image to the display.
     */
    RHIResult<void> present(SwapchainHandle, uint32_t) override {
        ++presents;
        return {};
    }

    /**
     * @brief Returns the swapchain extent.
     * @return The result of the operation.
     */
    std::pair<uint32_t, uint32_t> swapchainExtent(SwapchainHandle) const override {
        return {800, 600};
    }

    /** @brief Stores the created textures. */
    int createdTextures = 0;
    /** @brief Stores the destroyed textures. */
    int destroyedTextures = 0;
    /** @brief Stores the destroyed buffers. */
    int destroyedBuffers = 0;
    /** @brief Stores the destroyed surfaces. */
    int destroyedSurfaces = 0;
    /** @brief Stores the destroyed swapchains. */
    int destroyedSwapchains = 0;
    /** @brief Stores the presents. */
    int presents = 0;
    /** @brief Stores the destroyed pipelines. */
    int destroyedPipelines = 0;
    /** @brief Stores the destroyed bind group layouts. */
    int destroyedBindGroupLayouts = 0;
    /** @brief Stores the destroyed bind groups. */
    int destroyedBindGroups = 0;
    /** @brief Stores the destroyed samplers. */
    int destroyedSamplers = 0;
    /** @brief Stores the submits. */
    int submits = 0;
    /** @brief Stores the indexed draw calls. */
    int indexedDrawCalls = 0;
    /** @brief Stores the draw vertex counts. */
    std::vector<uint32_t> drawVertexCounts;
    /** @brief Stores the events. */
    std::vector<CmdEvent> events; // von allen CommandLists dieses Backends

private:
    /** @brief Stores the next buffer. */
    uint32_t m_nextBuffer = 0;
    /** @brief Stores the next texture. */
    uint32_t m_nextTexture = 0;
    /** @brief Stores the next surface. */
    uint32_t m_nextSurface = 0;
    /** @brief Stores the next pipeline. */
    uint32_t m_nextPipeline = 0;
    /** @brief Stores the next bind group layout. */
    uint32_t m_nextBindGroupLayout = 0;
    /** @brief Stores the next bind group. */
    uint32_t m_nextBindGroup = 0;
    /** @brief Stores the next sampler. */
    uint32_t m_nextSampler = 0;
    /** @brief Stores the next swapchain. */
    uint32_t m_nextSwapchain = 0;
};

/** @brief Stores the data for the probe frame. */
struct ProbeFrame {
    /** @brief Stores the priority. */
    int priority = 0;
    /** @brief Stores the viewport width. */
    uint32_t viewportWidth = 0;
    /** @brief Stores the z order. */
    std::vector<int32_t> zOrder;
};

/**
 * @brief Provides the queue probe pass functionality.
 * @tparam QueueProbePass Template type used by this type.
 */
class QueueProbePass final : public rendergraph::RenderPass {
public:
    /**
     * @brief Creates an instance of queue probe pass.
     * @param frames The frames used by this operation.
     * @param descriptor The descriptor used by this operation.
     */
    QueueProbePass(std::vector<ProbeFrame>& frames,
                   RenderQueueDescriptor descriptor)
        : m_frames(frames), m_descriptor(descriptor) {}

    /**
     * @brief Configures the resources required by this object.
     * @param RenderGraphBuilder The render graph builder used by this operation.
     */
    void setup(rendergraph::RenderGraphBuilder&) override {}

    /**
     * @brief Executes the operation using the supplied context.
     * @param ctx The ctx used by this operation.
     */
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

    /**
     * @brief Returns the name.
     * @return The result of the operation.
     */
    const char* name() const override { return "QueueProbePass"; }
    /**
     * @brief Reports whether this pass uses a render queue.
     * @return The result of the operation.
     */
    bool usesRenderQueue() const override { return true; }
    /**
     * @brief Returns the queue descriptor.
     * @return The result of the operation.
     */
    RenderQueueDescriptor queueDescriptor() const override {
        return m_descriptor;
    }

private:
    /** @brief Stores the frames. */
    std::vector<ProbeFrame>& m_frames;
    /** @brief Stores the descriptor. */
    RenderQueueDescriptor m_descriptor;
};

/**
 * @brief Performs the make item operation.
 * @param layer The layer used by this operation.
 * @param zIndex The z index used by this operation.
 * @param depth The depth used by this operation.
 * @param material The material used by this operation.
 * @return The result of the operation.
 */
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
        .sortMode = SortMode::backToFront,
    };

    auto result = queues.build(items, std::span<const RenderQueueDescriptor>(&descriptor, 1),
                               0b001);

    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0].size(), 2u);
    EXPECT_EQ(result[0][0].zIndex, 10);
    EXPECT_EQ(result[0][1].zIndex, 20);

    auto batches = batch_builder::build(result[0]);
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
/**
 * @brief Provides the present write pass functionality.
 * @tparam PresentWritePass Template type used by this type.
 */
class PresentWritePass final : public rendergraph::RenderPass {
public:
    /**
     * @brief Creates an instance of present write pass.
     * @param layout The layout used by this operation.
     * @param firstWrites The first writes used by this operation.
     * @param legacyImport The legacy import used by this operation.
     */
    PresentWritePass(TextureLayout layout, std::vector<bool>* firstWrites,
                     bool legacyImport = false)
        : m_layout(layout), m_firstWrites(firstWrites),
          m_legacyImport(legacyImport) {}

    /**
     * @brief Configures the resources required by this object.
     * @param builder The builder used by this operation.
     */
    void setup(rendergraph::RenderGraphBuilder& builder) override {
        if (m_legacyImport) {
            // Altes Muster: Pass importiert das Swapchain-Image selbst.
            rendergraph::TextureResourceDesc desc{
                .width = 64,
                .height = 64,
                .format = TextureFormat::bgrA8Unorm,
                .usage = TextureUsage::renderTarget,
                .debugName = "LegacyImport",
            };
            m_target = builder.write(
                builder.importTexture(builder.presentTarget(), desc), m_layout);
        } else {
            m_target = builder.write(builder.presentTargetResource(), m_layout);
        }
    }

    /**
     * @brief Executes the operation using the supplied context.
     * @param ctx The ctx used by this operation.
     * @param cmd The cmd used by this operation.
     */
    void execute(rendergraph::RenderContext& ctx, CommandList& cmd) override {
        const bool first = ctx.isFirstWrite(m_target);
        if (m_firstWrites)
            m_firstWrites->push_back(first);

        if (m_layout == TextureLayout::transferDst) {
            cmd.clearTexture(ctx.resolveTexture(m_target), ClearColor{});
            return;
        }
        std::optional<ClearColor> clear;
        if (first)
            clear = ClearColor{};
        cmd.beginRendering(ctx.resolveTexture(m_target), std::nullopt, clear);
        cmd.endRendering();
    }

    /**
     * @brief Returns the name.
     * @return The result of the operation.
     */
    const char* name() const override { return "PresentWritePass"; }

private:
    /** @brief Stores the layout. */
    TextureLayout m_layout;
    /** @brief Stores the first writes. */
    std::vector<bool>* m_firstWrites;
    /** @brief Stores the legacy import. */
    bool m_legacyImport;
    /** @brief Stores the target. */
    rendergraph::ResourceHandle m_target;
};

/**
 * @brief Provides the present probe pass functionality.
 * @tparam PresentProbePass Template type used by this type.
 */
class PresentProbePass final : public rendergraph::RenderPass {
public:
    /**
     * @brief Creates an instance of present probe pass.
     * @param sawValidResource The saw valid resource used by this operation.
     */
    explicit PresentProbePass(bool* sawValidResource)
        : m_saw(sawValidResource) {}
    /**
     * @brief Configures the resources required by this object.
     * @param builder The builder used by this operation.
     */
    void setup(rendergraph::RenderGraphBuilder& builder) override {
        *m_saw = builder.presentTargetResource().valid();
    }
    /**
     * @brief Executes the operation using the supplied context.
     * @param RenderContext The render context used by this operation.
     */
    void execute(rendergraph::RenderContext&, CommandList&) override {}
    /**
     * @brief Returns the name.
     * @return The result of the operation.
     */
    const char* name() const override { return "PresentProbePass"; }

private:
    /** @brief Stores the saw. */
    bool* m_saw;
};

/**
 * @brief Performs the make present execution operation.
 * @param view The view used by this operation.
 * @return The result of the operation.
 */
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
        TextureLayout::colorAttachment, &firstWrites));
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::colorAttachment, &firstWrites));

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
    EXPECT_EQ(backend.events[0].kind, K::transition);
    EXPECT_EQ(backend.events[0].from, TextureLayout::undefined);
    EXPECT_EQ(backend.events[0].to, TextureLayout::colorAttachment);
    EXPECT_EQ(backend.events[1].kind, K::beginRendering);
    EXPECT_TRUE(backend.events[1].clear);   // erster Schreiber cleart
    EXPECT_EQ(backend.events[2].kind, K::beginRendering);
    EXPECT_FALSE(backend.events[2].clear);  // zweiter laedt
    EXPECT_EQ(backend.events[3].kind, K::transition);
    EXPECT_EQ(backend.events[3].from, TextureLayout::colorAttachment);
    EXPECT_EQ(backend.events[3].to, TextureLayout::present);
}

TEST(RenderGraphTest, ImportsOfTheSameImageAreDeduplicated) {
    // Altes Muster: Passes importieren das Present-Target selbst (zusaetzlich
    // zum zentralen Import des Graphen). Muss trotzdem denselben Entry
    // ergeben, also dieselbe Ereignisfolge wie oben.
    ::NullBackend backend;
    rendergraph::RenderGraph graph(backend);
    std::vector<bool> firstWrites;
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::colorAttachment, &firstWrites, /*legacyImport=*/true));
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::colorAttachment, &firstWrites, /*legacyImport=*/true));

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
        TextureLayout::transferDst, &firstWrites));
    graph.addPass(std::make_unique<PresentWritePass>(
        TextureLayout::colorAttachment, &firstWrites));

    View view;
    view.viewport = Viewport{0, 0, 640, 480};
    const auto execution = makePresentExecution(view);
    ASSERT_TRUE(graph.compile(execution));
    ASSERT_TRUE(graph.execute(execution));

    using K = CmdEvent::Kind;
    ASSERT_EQ(backend.events.size(), 5u);
    EXPECT_EQ(backend.events[0].kind, K::transition);
    EXPECT_EQ(backend.events[0].to, TextureLayout::transferDst);
    EXPECT_EQ(backend.events[1].kind, K::clearTexture);
    EXPECT_EQ(backend.events[2].kind, K::transition);
    EXPECT_EQ(backend.events[2].from, TextureLayout::transferDst);
    EXPECT_EQ(backend.events[2].to, TextureLayout::colorAttachment);
    EXPECT_EQ(backend.events[3].kind, K::beginRendering);
    EXPECT_FALSE(backend.events[3].clear);
    EXPECT_EQ(backend.events[4].to, TextureLayout::present);
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

TEST(Renderer2DTest, SubmitsQuadCircleAndLinePrimitivesToRenderer) {
    Renderer renderer;
    Renderer2D::setActiveRenderer(&renderer);

    Renderer2D::beginScene();
    Renderer2D::submitQuad(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 0.0f)),
                           glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    Renderer2D::submitCircle(glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 4.0f, 0.0f)),
                            0.75f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    Renderer2D::submitLine(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(7.0f, 0.0f, 0.0f),
                          glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    Renderer2D::endScene();

    ASSERT_EQ(renderer.itemPool().size(), 3u);
    EXPECT_EQ(renderer.itemPool()[0].indexCount, 6u);
    EXPECT_EQ(renderer.itemPool()[1].indexCount, 6u);
    EXPECT_EQ(renderer.itemPool()[2].indexCount, 2u);
    EXPECT_EQ(Renderer2D::queuedItemCount(), 0u);

    Renderer2D::setActiveRenderer(nullptr);
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
                                      .sortMode = SortMode::backToFront}));

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

TEST(QuadRenderPassTest, CompilesShaderAndDrawsSceneQuad) {
    axiom::VFS::shutdown();
    /** @brief Stores the data for the VFS cleanup. */
    struct VfsCleanup {
        /**
         * @brief Releases resources owned by VfsCleanup.
         */
        ~VfsCleanup() { axiom::VFS::shutdown(); }
    /** @brief Stores the VFS cleanup. */
    } vfsCleanup;
    ASSERT_TRUE(axiom::VFS::mountPath("engine://", AXIOM_ENGINE_ASSET_PATH));

    auto backend = std::make_unique<NullBackend>();
    NullBackend* backendProbe = backend.get();
    Renderer renderer;
    ASSERT_TRUE(renderer.init(
        std::move(backend),
        RendererServicesDesc{
            .initializeAssetManager = false,
            .assetWorkerThreadCount = 0,
            .initializeBindlessTextureHeap = false,
        }));

    constexpr SurfaceHandle surface{1, 1};
    auto pass = std::make_unique<passes::QuadRenderPass>();
    ASSERT_TRUE(pass->initialize(renderer, surface, TextureFormat::bgrA8Unorm));
    renderer.addPass(std::move(pass));

    View view;
    view.target.kind = RenderTargetKind::swapchain;
    view.target.surface = surface;
    view.viewport = Viewport{0, 0, 640, 480};
    (void)renderer.registerView(view);

    RenderItem item;
    item.layer = 1u;
    item.transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.25f, -0.5f, 0.0f)) *
                     glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 1.0f));
    item.color = glm::vec4(0.2f, 0.7f, 0.9f, 1.0f);
    renderer.submitItem(item);

    ASSERT_TRUE(renderer.renderFrame());
    ASSERT_EQ(backendProbe->drawVertexCounts.size(), 1u);
    EXPECT_EQ(backendProbe->drawVertexCounts[0], 6u);
}

TEST(Render2DSystemTest, UpdatesCameraViewAndSubmitsQuadEntities) {
    auto backend = std::make_unique<NullBackend>();
    Renderer renderer;
    ASSERT_TRUE(renderer.init(
        std::move(backend),
        RendererServicesDesc{
            .initializeAssetManager = false,
            .assetWorkerThreadCount = 0,
            .initializeBindlessTextureHeap = false,
        }));

    View view;
    view.viewport = Viewport{0, 0, 800, 600};
    const ViewID viewId = renderer.registerView(view);
    axiom::Render2DSystem system(renderer, viewId, 800, 600);

    axiom::Scene scene;
    auto camera = scene.createEntity("Camera");
    camera.addComponent<axiom::CameraComponent>(true).OrthographicSize = 100.0f;
    camera.getComponent<axiom::TransformComponent>().Translation =
        glm::vec3(10.0f, 20.0f, 0.0f);

    auto quad = scene.createEntity("Quad");
    auto &quadTransform = quad.getComponent<axiom::TransformComponent>();
    quadTransform.Translation = glm::vec3(2.0f, 3.0f, 0.0f);
    quadTransform.Scale = glm::vec3(12.0f, 8.0f, 1.0f);
    const glm::vec4 color(0.7f, 0.2f, 0.1f, 1.0f);
    quad.addComponent<axiom::QuadRendererComponent>(color);

    system.render(scene, 0.0);

    ASSERT_EQ(renderer.itemPool().size(), 1u);
    EXPECT_EQ(renderer.itemPool()[0].transform, quadTransform.getTransform());
    EXPECT_EQ(renderer.itemPool()[0].color, color);

    const View *updatedView = renderer.view(viewId);
    ASSERT_NE(updatedView, nullptr);
    EXPECT_FLOAT_EQ(updatedView->viewMatrix[3][0], -10.0f);
    EXPECT_FLOAT_EQ(updatedView->viewMatrix[3][1], -20.0f);
    EXPECT_NE(updatedView->projectionMatrix, glm::mat4(1.0f));
}
