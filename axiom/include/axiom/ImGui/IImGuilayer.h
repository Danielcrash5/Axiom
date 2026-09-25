#pragma once

#include "axiom/core/Layer.h"
#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"
#include "axiom/platform/Window.h"
#include <SDL3/SDL_events.h>
#include <imgui.h>
#include <memory>

namespace axiom {

    namespace renderer {
        class Renderer;
    }

    enum class DockspaceType {
        None = 0,            // kein Dockspace, Fenster verhält sich normal
        Fullscreen = 1 << 0, // Dockspace füllt das gesamte Fenster aus, mit
                             // Ränder oder Titelbar
        Passthrough =
            1 << 1 // Dockspace ist durchsichtig, Engine kann normal in Fenster
                   // Rendern, ImGui-Elemente werden darüber gezeichnet
    };

    class IImGuiLayer : public Layer {
      public:
        IImGuiLayer() : Layer("ImGui"), m_DockspaceType(DockspaceType::None) {}
        virtual ~IImGuiLayer() = default;
        virtual void Begin() = 0;
        virtual void End() = 0;

        inline void SetDockspaceType(DockspaceType type) {
            m_DockspaceType = type;
        }

        // Wird vom Window VOR dem Weiterreichen an den EventBus aufgerufen.
        // true = Event wurde von der UI konsumiert (nicht ans Spiel geben).
        virtual bool ProcessEvent(const SDL_Event &event) { return false; }

        // Zeichnet und praesentiert aus dem Dockspace herausgezogene
        // ImGui-Fenster als eigene OS-Fenster (ImGuiConfigFlags_ViewportsEnable).
        // MUSS NACH dem Praesentieren des Hauptfensters aufgerufen werden -
        // die Sekundaerfenster haben ihre eigenen Swapchains, komplett am
        // RenderGraph vorbei. No-op, wenn Viewports nicht aktiviert sind.
        virtual void RenderAdditionalViewports() {}

        // Erzeugt den Layer passend zum Renderer-Backend und registriert
        // dessen RenderPass. nullptr bei Fehler (Renderer laeuft dann ohne UI).
        // Muss vor dem Renderer zerstoert werden.
        static std::unique_ptr<IImGuiLayer> Create(Window &window,
                                                   renderer::Renderer &renderer);

        /*ImFont* GetDefaultFont() const { return m_DefaultFont; }
        ImFont* GetMonospaceFont() const { return m_MonospaceFont; }*/

      protected:
        DockspaceType m_DockspaceType; /*
         ImFont* m_DefaultFont = nullptr;
         ImFont* m_MonospaceFont = nullptr;*/
    };
} // namespace axiom