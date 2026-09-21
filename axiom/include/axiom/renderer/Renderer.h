#pragma once
#include <memory>
#include <vector>
#include <span>
#include <functional>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <utility>
#include "rhi/IRHIBackend.h"
#include "BindlessTextureHeap.h"
#include "MaterialSystem.h"
#include "RenderLayerRegistry.h"
#include "RenderItem.h"
#include "RenderQueue.h"
#include "ShaderRegistry.h"
#include "View.h"
#include "rendergraph/RenderGraph.h"

namespace axiom::renderer {

// Windowing-neutral: die Anwendung liefert Instance-Extensions und eine
// Funktion, die aus einer nativen Instance eine native Surface baut (z.B.
// via SDL_Vulkan_CreateSurface). Damit kennt axiom_renderer kein
// SDL3/GLFW/Win32 - das bleibt Anwendungssache (siehe adapters/SDL3Window.h).
struct WindowSurfaceDesc {
    std::vector<const char*> requiredInstanceExtensions;
    std::function<rhi::RHIResult<void*>(void* nativeInstance)> createSurface;
};

struct RendererServicesDesc {
    bool initializeAssetManager = true;
    size_t assetWorkerThreadCount = 2;
    bool initializeBindlessTextureHeap = true;
};

struct RendererInitDesc {
    WindowSurfaceDesc windowSurface;
    RendererServicesDesc services;
};

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    [[nodiscard]] rhi::RHIResult<void> init(const WindowSurfaceDesc& windowDesc);
    [[nodiscard]] rhi::RHIResult<void> init(const RendererInitDesc& initDesc);
    [[nodiscard]] rhi::RHIResult<void> init(
        std::unique_ptr<rhi::IRHIBackend> backend,
        const RendererServicesDesc& services = {});
    void shutdown();

    [[nodiscard]] bool initialized() const { return m_backend != nullptr; }

    [[nodiscard]] rhi::IRHIBackend& backend() { return *m_backend; }
    [[nodiscard]] const rhi::IRHIBackend& backend() const { return *m_backend; }

    [[nodiscard]] ShaderRegistry& shaderRegistry() { return m_shaders; }
    [[nodiscard]] const ShaderRegistry& shaderRegistry() const { return m_shaders; }

    [[nodiscard]] MaterialSystem& materialSystem() { return *m_materialSystem; }
    [[nodiscard]] const MaterialSystem& materialSystem() const { return *m_materialSystem; }

    [[nodiscard]] BindlessTextureHeap& bindlessTextureHeap() { return m_bindlessTextures; }
    [[nodiscard]] const BindlessTextureHeap& bindlessTextureHeap() const { return m_bindlessTextures; }
    [[nodiscard]] bool bindlessTextureHeapAvailable() const {
        return m_bindlessTextures.initialized();
    }

    [[nodiscard]] RenderLayerRegistry& layerRegistry() { return m_layerRegistry; }
    [[nodiscard]] const RenderLayerRegistry& layerRegistry() const { return m_layerRegistry; }

    [[nodiscard]] rendergraph::RenderGraph& renderGraph() { return *m_renderGraph; }
    [[nodiscard]] const rendergraph::RenderGraph& renderGraph() const { return *m_renderGraph; }

    // Surface des Hauptfensters aus init(). Zusaetzliche Surfaces (z.B. fuer
    // ImGui-Viewport-Fenster) werden spaeter direkt ueber backend().createSurface(...)
    // angelegt - Renderer haelt hier bewusst nur die des Hauptfensters vor.
    [[nodiscard]] rhi::SurfaceHandle mainSurface() const { return m_mainSurface; }

    // --- Item-Pool (Phase 4) ---
    // ECS-Systeme (RenderSubmissionSystem-Ableitungen) rufen das pro Entity
    // auf - roh, unsortiert. Sortierung/Batching passiert spaeter im Graph
    // (Phase 7), nicht hier (siehe Design-Doc Abschnitt 10).
    void beginFrame() { clearItemPool(); }
    void submitItem(RenderItem item) { m_itemPool.push_back(std::move(item)); }
    [[nodiscard]] std::span<const RenderItem> itemPool() const { return m_itemPool; }
    void clearItemPool() { m_itemPool.clear(); } // zu Frame-Beginn aufrufen

    // --- Passes / Views / Frame-Ausfuehrung ---
    void addPass(std::unique_ptr<rendergraph::RenderPass> pass);
    void clearPasses();

    [[nodiscard]] ViewID registerView(const View& view);
    bool updateView(ViewID id, const View& view);
    bool removeView(ViewID id);
    [[nodiscard]] const View* view(ViewID id) const;
    [[nodiscard]] size_t viewCount() const { return m_views.size(); }

    [[nodiscard]] rhi::RHIResult<void> renderFrame();

private:
    [[nodiscard]] rhi::RHIResult<void>
    initServices(const RendererServicesDesc& services);
    [[nodiscard]] rhi::RHIResult<void>
    finishBackendInit(const RendererInitDesc& initDesc);
    [[nodiscard]] std::vector<ViewID> sortedViewIds() const;
    // Legt bei Bedarf einen Swapchain fuer diese Surface an (einmalig,
    // danach wiederverwendet) - eine Surface kann mehrere Views bedienen,
    // aber jede Surface hat genau einen Swapchain.
    [[nodiscard]] rhi::RHIResult<rhi::SwapchainHandle>
    getOrCreateSwapchain(rhi::SurfaceHandle surface, uint32_t width, uint32_t height);

    std::unique_ptr<rhi::IRHIBackend> m_backend;
    std::unique_ptr<rendergraph::RenderGraph> m_renderGraph;
    std::unique_ptr<MaterialSystem> m_materialSystem;
    ShaderRegistry m_shaders;
    BindlessTextureHeap m_bindlessTextures;
    RenderLayerRegistry m_layerRegistry;
    RenderQueueSystem m_renderQueues;
    rhi::SurfaceHandle m_mainSurface;
    std::vector<std::pair<rhi::SurfaceHandle, rhi::SwapchainHandle>> m_swapchains;
    std::vector<RenderItem> m_itemPool;
    std::unordered_map<ViewID, View> m_views;
    ViewID m_nextViewId = 1;
    bool m_ownsAssetManager = false;
};

} // namespace axiom::renderer
