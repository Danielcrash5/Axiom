/**
 * @file axiom/include/axiom/core/Application.h
 * @brief Declares the public interface for the application.
 */

// ===== axiom/include/axiom/core/Application.h =====
#pragma once
#include "axiom/core/LayerStack.h"
#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"
#include "axiom/input/Input.h"
#include "axiom/input/InputSystem.h"
#include "axiom/platform/Window.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/adapters/SDL3Window.h"
#include "axiom/renderer/passes/ClearScreenPass.h"
#include "axiom/ImGui/IImGuiLayer.h"
#include "axiom/ImGui/ImGuiPanelManager.h"
#include "axiom/ecs/Scene.h"
#include "axiom/ecs/SystemManager.h"
#include <memory>
#include <string>
#include <vector>

namespace axiom {

    /**
     * @brief Owns the application lifecycle and coordinates the engine systems.
     * @tparam Application Template type used by this type.
     */
    class Application {
      public:
        /**
         * @brief Creates an instance of application.
         * @param appName The app name used by this operation.
         * @param width The width used by this operation.
         * @param height The height used by this operation.
         */
        Application(std::string appName = "Axiom Engine", uint32_t width = 1280,
                    uint32_t height = 720);
        /**
         * @brief Releases resources owned by Application.
         */
        virtual ~Application() = default;

        /**
         * @brief Runs the main application loop.
         */
        void run();

        /**
         * @brief Adds layer.
         * @param layer The layer used by this operation.
         */
        void pushLayer(std::unique_ptr<Layer> layer) {
            m_layerStack.pushLayer(std::move(layer), m_eventBus);
        }

        /**
         * @brief Adds overlay.
         * @param overlay The overlay used by this operation.
         */
        void pushOverlay(std::unique_ptr<Layer> overlay) {
            m_layerStack.pushOverlay(std::move(overlay), m_eventBus);
        }

        /**
         * @brief Removes layer.
         * @param layer The layer used by this operation.
         */
        void popLayer(Layer *layer) { m_layerStack.popLayer(layer); }

        /**
         * @brief Removes overlay.
         * @param overlay The overlay used by this operation.
         */
        void popOverlay(Layer *overlay) { m_layerStack.popOverlay(overlay); }

        /**
         * @brief Adds ImGui panel.
         * @param panel The panel used by this operation.
         */
        void addImGuiPanel(const std::shared_ptr<IImGuiPanel> &panel) {
            m_imGuiPanelManager.addPanel(panel);
        }

        /**
         * @brief Removes ImGui panel.
         * @param name The name used by this operation.
         */
        void removeImGuiPanel(const std::string &name) {
            m_imGuiPanelManager.removePanel(name);
        }

        /**
         * @brief Returns the requested value.
         * @return The result of the operation.
         */
        static Application &get() { return *sInstance; }

        /**
         * @brief Runs the main application loop.
         * @param argc The argc used by this operation.
         * @param argv The argv used by this operation.
         */
        void run(int argc, char **argv);

        /**
         * @brief Returns the command line args.
         * @return The result of the operation.
         */
        const std::vector<std::string> &getCommandLineArgs() const {
            return m_CommandLineArgs;
        }

        /**
         * @brief Returns the event bus.
         * @return The result of the operation.
         */
        EventBus &getEventBus() { return m_eventBus; }

        /**
         * @brief Returns the window.
         * @return The result of the operation.
         */
        std::shared_ptr<Window> getWindow() { return m_window; }

        /**
         * @brief Returns the width.
         * @return The result of the operation.
         */
        uint32_t getWidth() const { return m_width; }

        /**
         * @brief Returns the heigth.
         * @return The result of the operation.
         */
        uint32_t getHeigth() const { return m_height; }

        /**
         * @brief Returns the input.
         * @return The result of the operation.
         */
        Input &getInput() { return m_input; }

        /**
         * @brief Returns the input system.
         * @return The result of the operation.
         */
        InputSystem &getInputSystem() { return m_inputSystem; }

        /**
         * @brief Returns the scene.
         * @return The result of the operation.
         */
        Scene &getScene() { return *m_Scenes.front(); }

        /**
         * @brief Creates scene.
         * @return The result of the operation.
         */
        Scene &createScene() {
            m_Scenes.push_back(std::make_unique<Scene>());
            return *m_Scenes.back();
        }

        /**
         * @brief Returns the scenes.
         * @return The result of the operation.
         */
        std::vector<std::unique_ptr<Scene>> &getScenes() { return m_Scenes; }

        /**
         * @brief Registers system.
         * @tparam T Template type used by this operation.
         * @param args The args used by this operation.
         * @return The result of the operation.
         */
        template <typename T, typename... Args>
        std::shared_ptr<T> registerSystem(Args &&...args) {
            return m_systemManager.addSystem<T>(std::forward<Args>(args)...);
        }

        /**
         * @brief Returns the system manager.
         * @return The result of the operation.
         */
        SystemManager &getSystemManager() { return m_systemManager; }

        /**
         * @brief Returns the system manager.
         * @return The result of the operation.
         */
        const SystemManager &getSystemManager() const {
            return m_systemManager;
        }

        /**
         * @brief Returns the renderer.
         * @return The result of the operation.
         */
        renderer::Renderer &getRenderer() { return *m_renderer; }
        /**
         * @brief Returns the main view ID.
         * @return The result of the operation.
         */
        renderer::ViewID getMainViewId() const { return m_mainViewId; }

      protected:
        /**
         * @brief Handles the init callback.
         */
        virtual void onInit() {}
        /**
         * @brief Handles the shutdown callback.
         */
        virtual void onShutdown() {}

        /**
         * @brief Handles the pre update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onPreUpdate(double dt) {}
        /**
         * @brief Handles the post update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onPostUpdate(double dt) {}
        /**
         * @brief Handles the fixed update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onFixedUpdate(double dt) {}
        /**
         * @brief Handles the update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onUpdate(double dt) {}

        /**
         * @brief Handles the render callback.
         * @param alpha The alpha used by this operation.
         */
        virtual void onRender(double alpha) {}
        /**
         * @brief Handles the ImGui render callback.
         */
        virtual void onImGuiRender() {}

        /**
         * @brief Closes the resource and releases its handles.
         */
        void close() { m_running = false; }

        /** @brief Stores the ImGui layer. */
        std::unique_ptr<IImGuiLayer> m_ImGuiLayer;

        /** @brief Stores the scenes. */
        std::vector<std::unique_ptr<Scene>> m_Scenes;
        /** @brief Stores the command line args. */
        std::vector<std::string> m_CommandLineArgs;

      private:
        /**
         * @brief Runs the application's main loop.
         */
        void mainLoop();
        /**
         * @brief Initializes the subsystem.
         */
        void init();
        /**
         * @brief Shuts down the subsystem.
         */
        void shutdown();

        /**
         * @brief Updates the application systems for the current frame.
         */
        void mainUpdate();
        /**
         * @brief Updates state before the main update step.
         * @param dt The dt used by this operation.
         */
        void preUpdate(double dt);
        /**
         * @brief Performs work after the main update step.
         * @param dt The dt used by this operation.
         */
        void postUpdate(double dt);
        /**
         * @brief Updates state at a fixed time interval.
         * @param dt The dt used by this operation.
         */
        void fixedUpdate(double dt);
        /**
         * @brief Updates the object state.
         * @param dt The dt used by this operation.
         */
        void update(double dt);

        /**
         * @brief Renders the current frame.
         * @param alpha The alpha used by this operation.
         */
        void render(double alpha);
        /**
         * @brief Performs the ImGui render operation.
         */
        void imGuiRender();

        /**
         * @brief Handles the window close callback.
         * @param e The e used by this operation.
         * @return The result of the operation.
         */
        bool onWindowClose(WindowCloseEvent &e) {
            close();
            return true;
        }

        /**
         * @brief Handles the window resize callback.
         * @param e The e used by this operation.
         * @return The result of the operation.
         */
        bool onWindowResize(WindowResizeEvent &e) {
            m_width = e.width;
            m_height = e.height;
            if (m_renderer && m_mainViewId != renderer::kInvalidViewID) {
                if (const renderer::View *current = m_renderer->view(m_mainViewId)) {
                    renderer::View updated = *current;
                    updated.viewport.width = e.width;
                    updated.viewport.height = e.height;
                    m_renderer->updateView(m_mainViewId, updated);
                }
            }
            m_systemManager.onViewportResize(e.width, e.height);
            return false;
        }

      private:
        /** @brief Stores the running. */
        bool m_running = true;
        /** @brief Stores the window. */
        std::shared_ptr<Window> m_window;
        /** @brief Stores the app name. */
        std::string m_appName;

        /** @brief Stores the renderer. */
        std::unique_ptr<renderer::Renderer> m_renderer;
        /** @brief Stores the main view ID. */
        renderer::ViewID m_mainViewId = renderer::kInvalidViewID;

        /** @brief Stores the height. */
        uint32_t m_width, m_height;

        /** @brief Stores the singleton application instance. */
        static Application *sInstance;

        /** @brief Stores the input system. */
        InputSystem m_inputSystem;
        /** @brief Stores the input. */
        Input m_input;

        /** @brief Stores the ImGui panel manager. */
        ImGuiPanelManager m_imGuiPanelManager;

        /** @brief Stores the layer stack. */
        LayerStack m_layerStack;
        /** @brief Stores the event bus. */
        EventBus m_eventBus;

        /** @brief Stores the system manager. */
        SystemManager m_systemManager;
    };

} // namespace axiom

/** @brief Provides the GetMainEventBus macro. */
#define GetMainEventBus() axiom::Application::get().getEventBus()
/** @brief Provides the GetMainWindow macro. */
#define GetMainWindow() axiom::Application::get().getWindow()
/** @brief Provides the GetMainInput macro. */
#define GetMainInput() axiom::Application::get().getInput()
/** @brief Provides the GetMainInputSystem macro. */
#define GetMainInputSystem() axiom::Application::get().getInputSystem()
