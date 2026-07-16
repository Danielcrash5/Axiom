#pragma once
#include <memory>
#include <vector>
#include <span>
#include <functional>
#include "rhi/IRHIBackend.h"
#include "RenderItem.h"

namespace axiom::renderer {

// Windowing-neutral: die Anwendung liefert Instance-Extensions und eine
// Funktion, die aus einer nativen Instance eine native Surface baut (z.B.
// via SDL_Vulkan_CreateSurface). Damit kennt axiom_renderer kein
// SDL3/GLFW/Win32 - das bleibt Anwendungssache (siehe adapters/SDL3Window.h).
struct WindowSurfaceDesc {
    std::vector<const char*> requiredInstanceExtensions;
    std::function<rhi::RHIResult<void*>(void* nativeInstance)> createSurface;
};

class Renderer {
public:
    [[nodiscard]] rhi::RHIResult<void> init(const WindowSurfaceDesc& windowDesc);

    [[nodiscard]] rhi::IRHIBackend& backend() { return *m_backend; }
    [[nodiscard]] const rhi::IRHIBackend& backend() const { return *m_backend; }

    // Surface des Hauptfensters aus init(). Zusaetzliche Surfaces (z.B. fuer
    // ImGui-Viewport-Fenster) werden spaeter direkt ueber backend().createSurface(...)
    // angelegt - Renderer haelt hier bewusst nur die des Hauptfensters vor.
    [[nodiscard]] rhi::SurfaceHandle mainSurface() const { return m_mainSurface; }

    // --- Item-Pool (Phase 4) ---
    // ECS-Systeme (RenderSubmissionSystem-Ableitungen) rufen das pro Entity
    // auf - roh, unsortiert. Sortierung/Batching passiert spaeter im Graph
    // (Phase 7), nicht hier (siehe Design-Doc Abschnitt 10).
    void submitItem(RenderItem item) { m_itemPool.push_back(std::move(item)); }
    [[nodiscard]] std::span<const RenderItem> itemPool() const { return m_itemPool; }
    void clearItemPool() { m_itemPool.clear(); } // zu Frame-Beginn aufrufen

    // Waechst in Phase 8 (registerView/updateView/removeView, renderFrame).

private:
    std::unique_ptr<rhi::IRHIBackend> m_backend;
    rhi::SurfaceHandle m_mainSurface;
    std::vector<RenderItem> m_itemPool;
};

} // namespace axiom::renderer

