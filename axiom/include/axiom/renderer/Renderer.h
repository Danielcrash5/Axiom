/**
 * @file axiom/include/axiom/renderer/Renderer.h
 * @brief Declares the public interface for the renderer.
 */

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
/** @brief Stores the data for the window surface desc. */
struct WindowSurfaceDesc {
    /** @brief Stores the required instance extensions. */
    std::vector<const char*> requiredInstanceExtensions;
    std::function<rhi::RHIResult<void*>(void* nativeInstance)> createSurface;
};

/** @brief Stores the data for the renderer services desc. */
struct RendererServicesDesc {
    /** @brief Stores the initialize asset manager. */
    bool initializeAssetManager = true;
    /** @brief Stores the asset worker thread count. */
    size_t assetWorkerThreadCount = 2;
    /** @brief Stores the initialize bindless texture heap. */
    bool initializeBindlessTextureHeap = true;
};

/** @brief Stores the data for the renderer init desc. */
struct RendererInitDesc {
    /** @brief Stores the window surface. */
    WindowSurfaceDesc windowSurface;
    /** @brief Stores the services. */
    RendererServicesDesc services;
};

/**
 * @brief Coordinates frame rendering through the selected graphics backend.
 * @tparam Renderer Template type used by this type.
 */
class Renderer {
public:
    /**
     * @brief Creates an instance of renderer.
     */
    Renderer() = default;
    /**
     * @brief Releases resources owned by Renderer.
     */
    ~Renderer();

    /**
     * @brief Creates an instance of renderer.
     * @param Renderer The renderer used by this operation.
     */
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    /**
     * @brief Creates an instance of renderer.
     */
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /**
     * @brief Initializes the subsystem.
     * @param windowDesc The window desc used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> init(const WindowSurfaceDesc& windowDesc);
    /**
     * @brief Initializes the subsystem.
     * @param initDesc The init desc used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> init(const RendererInitDesc& initDesc);
    /**
     * @brief Initializes the subsystem.
     * @param backend The backend used by this operation.
     * @param services The services used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> init(
        std::unique_ptr<rhi::IRHIBackend> backend,
        const RendererServicesDesc& services = {});
    /**
     * @brief Shuts down the subsystem.
     */
    void shutdown();

    /**
     * @brief Initializes ialized.
     * @return The result of the operation.
     */
    [[nodiscard]] bool initialized() const { return m_backend != nullptr; }

    /**
     * @brief Returns the backend.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::IRHIBackend& backend() { return *m_backend; }
    /**
     * @brief Returns the backend.
     * @return The result of the operation.
     */
    [[nodiscard]] const rhi::IRHIBackend& backend() const { return *m_backend; }

    /**
     * @brief Returns the shader registry.
     * @return The result of the operation.
     */
    [[nodiscard]] ShaderRegistry& shaderRegistry() { return m_shaders; }
    /**
     * @brief Returns the shader registry.
     * @return The result of the operation.
     */
    [[nodiscard]] const ShaderRegistry& shaderRegistry() const { return m_shaders; }

    /**
     * @brief Returns the material system.
     * @return The result of the operation.
     */
    [[nodiscard]] MaterialSystem& materialSystem() { return *m_materialSystem; }
    /**
     * @brief Returns the material system.
     * @return The result of the operation.
     */
    [[nodiscard]] const MaterialSystem& materialSystem() const { return *m_materialSystem; }

    /**
     * @brief Binds less texture heap.
     * @return The result of the operation.
     */
    [[nodiscard]] BindlessTextureHeap& bindlessTextureHeap() { return m_bindlessTextures; }
    /**
     * @brief Binds less texture heap.
     * @return The result of the operation.
     */
    [[nodiscard]] const BindlessTextureHeap& bindlessTextureHeap() const { return m_bindlessTextures; }
    /**
     * @brief Binds less texture heap available.
     * @return The result of the operation.
     */
    [[nodiscard]] bool bindlessTextureHeapAvailable() const {
        return m_bindlessTextures.initialized();
    }

    /**
     * @brief Returns the layer registry.
     * @return The result of the operation.
     */
    [[nodiscard]] RenderLayerRegistry& layerRegistry() { return m_layerRegistry; }
    /**
     * @brief Returns the layer registry.
     * @return The result of the operation.
     */
    [[nodiscard]] const RenderLayerRegistry& layerRegistry() const { return m_layerRegistry; }

    /**
     * @brief Renders graph.
     * @return The result of the operation.
     */
    [[nodiscard]] rendergraph::RenderGraph& renderGraph() { return *m_renderGraph; }
    /**
     * @brief Renders graph.
     * @return The result of the operation.
     */
    [[nodiscard]] const rendergraph::RenderGraph& renderGraph() const { return *m_renderGraph; }

    // Surface des Hauptfensters aus init(). Zusaetzliche Surfaces (z.B. fuer
    // ImGui-Viewport-Fenster) werden spaeter direkt ueber backend().createSurface(...)
    // angelegt - Renderer haelt hier bewusst nur die des Hauptfensters vor.
    /**
     * @brief Performs the main surface operation.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::SurfaceHandle mainSurface() const { return m_mainSurface; }

    // Legt (falls noetig) den Swapchain dieser Surface an und liefert sein
    // Format. Fuer Consumer, die Pipelines gegen das Present-Format bauen
    // muessen, BEVOR der erste renderFrame() lief (z.B. ImGui-Backend).
    /**
     * @brief Returns the surface format.
     * @param surface The surface used by this operation.
     * @param width The width used by this operation.
     * @param height The height used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::RHIResult<rhi::TextureFormat>
    surfaceFormat(rhi::SurfaceHandle surface, uint32_t width, uint32_t height);

    // Schaltet VSync fuer die Swapchain der uebergebenen Surface um (FIFO vs.
    // MAILBOX/IMMEDIATE). Synchron, wartet einmal auf das Device - nur bei
    // Nutzeraktion aufrufen (Settings-Menu), nicht pro Frame.
    /**
     * @brief Sets vsync.
     * @param surface The surface used by this operation.
     * @param enabled The enabled used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void> setVsync(rhi::SurfaceHandle surface,
                                                bool enabled);

    // --- Item-Pool (Phase 4) ---
    // ECS-Systeme (RenderSubmissionSystem-Ableitungen) rufen das pro Entity
    // auf - roh, unsortiert. Sortierung/Batching passiert spaeter im Graph
    // (Phase 7), nicht hier (siehe Design-Doc Abschnitt 10).
    /**
     * @brief Begins frame.
     */
    void beginFrame() { clearItemPool(); }
    /**
     * @brief Submits item.
     * @param item The item used by this operation.
     */
    void submitItem(RenderItem item) { m_itemPool.push_back(std::move(item)); }
    /**
     * @brief Performs the item pool operation.
     * @return The result of the operation.
     */
    [[nodiscard]] std::span<const RenderItem> itemPool() const { return m_itemPool; }
    /**
     * @brief Clears item pool.
     */
    void clearItemPool() { m_itemPool.clear(); } // zu Frame-Beginn aufrufen

    // --- Passes / Views / Frame-Ausfuehrung ---
    /**
     * @brief Adds pass.
     * @param pass The pass used by this operation.
     */
    void addPass(std::unique_ptr<rendergraph::RenderPass> pass);
    /**
     * @brief Adds pass first.
     * @param pass The pass used by this operation.
     */
    void addPassFirst(std::unique_ptr<rendergraph::RenderPass> pass);
    /**
     * @brief Clears passes.
     */
    void clearPasses();

    /**
     * @brief Registers view.
     * @param view The view used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] ViewID registerView(const View& view);
    /**
     * @brief Updates view.
     * @param id The ID used by this operation.
     * @param view The view used by this operation.
     * @return The result of the operation.
     */
    bool updateView(ViewID id, const View& view);
    /**
     * @brief Removes view.
     * @param id The ID used by this operation.
     * @return The result of the operation.
     */
    bool removeView(ViewID id);
    /**
     * @brief Returns the view.
     * @param id The ID used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] const View* view(ViewID id) const;
    /**
     * @brief Returns the view count.
     * @return The result of the operation.
     */
    [[nodiscard]] size_t viewCount() const { return m_views.size(); }

    /**
     * @brief Renders frame.
     */
    [[nodiscard]] rhi::RHIResult<void> renderFrame();

private:
    /**
     * @brief Initializes services.
     * @param services The services used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void>
    initServices(const RendererServicesDesc& services);
    /**
     * @brief Performs the finish backend init operation.
     * @param initDesc The init desc used by this operation.
     */
    [[nodiscard]] rhi::RHIResult<void>
    finishBackendInit(const RendererInitDesc& initDesc);
    /**
     * @brief Sorts ed view ids.
     * @return The result of the operation.
     */
    [[nodiscard]] std::vector<ViewID> sortedViewIds() const;
    // Legt bei Bedarf einen Swapchain fuer diese Surface an (einmalig,
    // danach wiederverwendet) - eine Surface kann mehrere Views bedienen,
    // aber jede Surface hat genau einen Swapchain.
    /**
     * @brief Returns the or create swapchain.
     * @param surface The surface used by this operation.
     * @param width The width used by this operation.
     * @param height The height used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] rhi::RHIResult<rhi::SwapchainHandle>
    getOrCreateSwapchain(rhi::SurfaceHandle surface, uint32_t width, uint32_t height);

    /** @brief Stores the backend. */
    std::unique_ptr<rhi::IRHIBackend> m_backend;
    /** @brief Stores the render graph. */
    std::unique_ptr<rendergraph::RenderGraph> m_renderGraph;
    /** @brief Stores the material system. */
    std::unique_ptr<MaterialSystem> m_materialSystem;
    /** @brief Stores the shaders. */
    ShaderRegistry m_shaders;
    /** @brief Stores the bindless textures. */
    BindlessTextureHeap m_bindlessTextures;
    /** @brief Stores the layer registry. */
    RenderLayerRegistry m_layerRegistry;
    /** @brief Stores the render queues. */
    RenderQueueSystem m_renderQueues;
    /** @brief Stores the main surface. */
    rhi::SurfaceHandle m_mainSurface;
    /** @brief Stores the swapchains. */
    std::vector<std::pair<rhi::SurfaceHandle, rhi::SwapchainHandle>> m_swapchains;
    /** @brief Stores the item pool. */
    std::vector<RenderItem> m_itemPool;
    /** @brief Stores the views. */
    std::unordered_map<ViewID, View> m_views;
    /** @brief Stores the next view ID. */
    ViewID m_nextViewId = 1;
    /** @brief Stores the owns asset manager. */
    bool m_ownsAssetManager = false;
};

} // namespace axiom::renderer
