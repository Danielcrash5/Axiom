/**
 * @file axiom/include/axiom/ImGui/IImGuilayer.h
 * @brief Declares the public interface for the i im guilayer.
 */

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

    /**
     * @brief Defines the available dockspace type values.
     * @tparam DockspaceType Template type used by this type.
     */
    enum class DockspaceType {
        /** @brief Represents the none value. */
        none = 0,            // kein Dockspace, Fenster verhält sich normal
        /** @brief Represents the fullscreen value. */
        fullscreen = 1 << 0, // Dockspace füllt das gesamte Fenster aus, mit
                             // Ränder oder Titelbar
        /** @brief Represents the passthrough value. */
        passthrough =
            1 << 1 // Dockspace ist durchsichtig, Engine kann normal in Fenster
                   // Rendern, ImGui-Elemente werden darüber gezeichnet
    };

    /**
     * @brief Defines the interface for the ImGui layer.
     * @tparam IImGuiLayer Template type used by this type.
     */
    class IImGuiLayer : public Layer {
      public:
        /**
         * @brief Creates an instance of i ImGui layer.
         */
        IImGuiLayer() : Layer("ImGui"), m_DockspaceType(DockspaceType::none) {}
        /**
         * @brief Releases resources owned by IImGuiLayer.
         */
        virtual ~IImGuiLayer() = default;
        /**
         * @brief Begins the operation.
         */
        virtual void begin() = 0;
        /**
         * @brief Ends the operation.
         */
        virtual void end() = 0;

        /**
         * @brief Sets dockspace type.
         * @param type The type used by this operation.
         */
        inline void setDockspaceType(DockspaceType type) {
            m_DockspaceType = type;
        }

        // Wird vom Window VOR dem Weiterreichen an den EventBus aufgerufen.
        // true = Event wurde von der UI konsumiert (nicht ans Spiel geben).
        /**
         * @brief Processes event.
         * @param event The event used by this operation.
         * @return The result of the operation.
         */
        virtual bool processEvent(const SDL_Event &event) { return false; }

        // Zeichnet und praesentiert aus dem Dockspace herausgezogene
        // ImGui-Fenster als eigene OS-Fenster (ImGuiConfigFlags_ViewportsEnable).
        // MUSS NACH dem Praesentieren des Hauptfensters aufgerufen werden -
        // die Sekundaerfenster haben ihre eigenen Swapchains, komplett am
        // RenderGraph vorbei. No-op, wenn Viewports nicht aktiviert sind.
        /**
         * @brief Renders additional viewports.
         */
        virtual void renderAdditionalViewports() {}

        // Erzeugt den Layer passend zum Renderer-Backend und registriert
        // dessen RenderPass. nullptr bei Fehler (Renderer laeuft dann ohne UI).
        // Muss vor dem Renderer zerstoert werden.
        /**
         * @brief Creates the requested resource.
         * @param window The window used by this operation.
         * @param renderer The renderer used by this operation.
         * @return The result of the operation.
         */
        static std::unique_ptr<IImGuiLayer> create(Window &window,
                                                   renderer::Renderer &renderer);

        /*ImFont* GetDefaultFont() const { return m_DefaultFont; }
        ImFont* GetMonospaceFont() const { return m_MonospaceFont; }*/

      protected:
        /** @brief Stores the dockspace type. */
        DockspaceType m_DockspaceType; /*
         ImFont* m_DefaultFont = nullptr;
         ImFont* m_MonospaceFont = nullptr;*/
    };
} // namespace axiom