#include <axiom/renderer/Renderer.h>
#include <axiom/assets/AssetManager.h>
#include <axiom/renderer/rhi/Backends.h>
#include <algorithm>

namespace axiom::renderer {

Renderer::~Renderer() { shutdown(); }

rhi::RHIResult<void> Renderer::init(const WindowSurfaceDesc& windowDesc) {
    RendererInitDesc initDesc;
    initDesc.windowSurface = windowDesc;
    return init(initDesc);
}

rhi::RHIResult<void> Renderer::init(const RendererInitDesc& initDesc) {
    shutdown();

    auto backendResult =
        rhi::createVulkanBackend(initDesc.windowSurface.requiredInstanceExtensions);
    if (!backendResult) {
        return std::unexpected(backendResult.error());
    }
    m_backend = std::move(*backendResult);

    if (auto result = finishBackendInit(initDesc); !result) {
        shutdown();
        return result;
    }

    return {};
}

rhi::RHIResult<void> Renderer::init(std::unique_ptr<rhi::IRHIBackend> backend,
                                    const RendererServicesDesc& services) {
    shutdown();
    if (!backend) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    m_backend = std::move(backend);
    if (auto result = initServices(services); !result) {
        shutdown();
        return result;
    }

    return {};
}

void Renderer::shutdown() {
    if (m_ownsAssetManager) {
        AssetManager::Shutdown();
        m_ownsAssetManager = false;
    }

    if (m_renderGraph) {
        m_renderGraph->clear();
    }
    m_renderGraph.reset();
    m_materialSystem.reset();
    m_bindlessTextures.shutdown();

    // Swapchains VOR ihrer Surface zerstoeren (Vulkan verlangt das) - siehe
    // VulkanBackend::destroySwapchain()/destroySurface()-Reihenfolge.
    if (m_backend) {
        for (auto& [surface, swapchain] : m_swapchains) {
            m_backend->destroySwapchain(swapchain);
        }
    }
    m_swapchains.clear();

    if (m_backend && m_mainSurface.valid()) {
        m_backend->destroySurface(m_mainSurface);
    }
    m_mainSurface = {};
    m_backend.reset();
    m_itemPool.clear();
    m_views.clear();
    m_nextViewId = 1;
}

rhi::RHIResult<void>
Renderer::finishBackendInit(const RendererInitDesc& initDesc) {
    if (!m_backend) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    if (initDesc.windowSurface.createSurface) {
        auto nativeSurfaceResult =
            initDesc.windowSurface.createSurface(m_backend->nativeInstanceHandle());
        if (!nativeSurfaceResult) {
            return std::unexpected(nativeSurfaceResult.error());
        }

        auto surfaceResult = m_backend->createSurface(*nativeSurfaceResult);
        if (!surfaceResult) {
            return std::unexpected(surfaceResult.error());
        }
        m_mainSurface = *surfaceResult;
    }

    return initServices(initDesc.services);
}

rhi::RHIResult<void>
Renderer::initServices(const RendererServicesDesc& services) {
    if (services.initializeAssetManager && !AssetManager::IsInitialized()) {
        AssetManager::Init(services.assetWorkerThreadCount);
        m_ownsAssetManager = true;
    }

    m_renderGraph = std::make_unique<rendergraph::RenderGraph>(*m_backend);
    m_materialSystem = std::make_unique<MaterialSystem>(*m_backend, m_shaders);

    if (services.initializeBindlessTextureHeap) {
        if (auto result = m_bindlessTextures.init(*m_backend); !result) {
            return std::unexpected(result.error());
        }
    }

    return {};
}

void Renderer::addPass(std::unique_ptr<rendergraph::RenderPass> pass) {
    if (!m_renderGraph) {
        return;
    }
    m_renderGraph->addPass(std::move(pass));
}

void Renderer::clearPasses() {
    if (!m_renderGraph) {
        return;
    }
    m_renderGraph->clear();
}

ViewID Renderer::registerView(const View& view) {
    const ViewID id = m_nextViewId++;
    m_views[id] = view;
    return id;
}

bool Renderer::updateView(ViewID id, const View& view) {
    auto it = m_views.find(id);
    if (it == m_views.end()) {
        return false;
    }
    it->second = view;
    return true;
}

bool Renderer::removeView(ViewID id) {
    return m_views.erase(id) > 0;
}

const View* Renderer::view(ViewID id) const {
    auto it = m_views.find(id);
    if (it == m_views.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<ViewID> Renderer::sortedViewIds() const {
    std::vector<ViewID> ids;
    ids.reserve(m_views.size());
    for (const auto& [id, view] : m_views) {
        (void)view;
        ids.push_back(id);
    }

    std::stable_sort(ids.begin(), ids.end(), [this](ViewID left, ViewID right) {
        return m_views.at(left).priority < m_views.at(right).priority;
    });
    return ids;
}

rhi::RHIResult<rhi::SwapchainHandle> Renderer::getOrCreateSwapchain(
    rhi::SurfaceHandle surface, uint32_t width, uint32_t height) {
    for (auto& [existingSurface, existingSwapchain] : m_swapchains) {
        if (existingSurface == surface) {
            return existingSwapchain;
        }
    }

    rhi::SwapchainDesc desc{
        .surface = surface,
        .width = width,
        .height = height,
        .vsync = true,
    };
    auto result = m_backend->createSwapchain(desc);
    if (!result) {
        return std::unexpected(result.error());
    }
    m_swapchains.emplace_back(surface, *result);
    return *result;
}

rhi::RHIResult<void> Renderer::renderFrame() {
    if (!m_backend || !m_renderGraph) {
        return std::unexpected(rhi::RHIError::InvalidDescriptor);
    }

    auto renderOneView = [this](const View* view) -> rhi::RHIResult<void> {
        // Fuer Views mit Swapchain-Target: VOR compile()/execute() das
        // aktuelle Swapchain-Image holen, damit Passes es waehrend setup()
        // ueber RenderGraphBuilder::presentTarget() importieren koennen
        // (siehe ClearScreenPass). Ohne diesen Schritt bleibt das Fenster
        // schwarz, weil nie in ein tatsaechliches Swapchain-Image gezeichnet
        // und nie praesentiert wird.
        rhi::TextureHandle presentTarget;
        std::optional<rhi::SwapchainHandle> swapchainToPresent;
        uint32_t presentImageIndex = 0;

        if (view && view->target.kind == RenderTargetKind::Swapchain &&
            view->target.surface.valid()) {
            uint32_t width = view->viewport.width > 0 ? view->viewport.width : 1;
            uint32_t height = view->viewport.height > 0 ? view->viewport.height : 1;

            auto swapchainResult = getOrCreateSwapchain(view->target.surface, width, height);
            if (!swapchainResult) {
                return std::unexpected(swapchainResult.error());
            }

            auto acquireResult = m_backend->acquireNextImage(*swapchainResult);
            if (!acquireResult) {
                return std::unexpected(acquireResult.error());
            }

            presentTarget = acquireResult->texture;
            presentImageIndex = acquireResult->imageIndex;
            swapchainToPresent = *swapchainResult;
        }

        rendergraph::RenderExecutionDesc execution{
            .view = view,
            .presentTarget = presentTarget,
        };

        auto compileResult = m_renderGraph->compile(execution);
        if (!compileResult) {
            return std::unexpected(compileResult.error());
        }

        auto descriptors = m_renderGraph->queueDescriptors();
        const RenderLayerMask visibleLayers =
            view ? view->visibleLayers : ~RenderLayerMask{0};
        auto queues =
            m_renderQueues.build(m_itemPool, descriptors, visibleLayers);
        execution.queuedItemsByPass = queues;

        if (auto result = m_renderGraph->execute(execution); !result) {
            return result;
        }

        if (swapchainToPresent) {
            return m_backend->present(*swapchainToPresent, presentImageIndex);
        }
        return {};
    };

    if (m_views.empty()) {
        auto result = renderOneView(nullptr);
        clearItemPool();
        return result;
    }

    for (ViewID id : sortedViewIds()) {
        const View* currentView = view(id);
        if (!currentView) {
            continue;
        }
        if (auto result = renderOneView(currentView); !result) {
            clearItemPool();
            return result;
        }
    }

    clearItemPool();
    return {};
}

} // namespace axiom::renderer
