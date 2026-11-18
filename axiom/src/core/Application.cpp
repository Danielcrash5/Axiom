/**
 * @file axiom/src/core/Application.cpp
 * @brief Implements the application module.
 */

// ===== axiom/src/core/Application.cpp =====
#include "axiom/core/Application.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Time.h"
#include "axiom/platform/Window.h"
#include "axiom/profiling/Profiler.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/ImGui/Panels/LogPanel.h"
#include "axiom/assets/VFS.h"
#include <vector>

namespace axiom {

    Application *Application::sInstance = nullptr;

    static double mLastFixedUpdate = 0.0f;
    static double mFixedUpdateInterval = 1.0 / 75.0;

    Application::Application(std::string appName, uint32_t width,
                             uint32_t height) {
        AXIOM_ASSERT(!sInstance, "Application already exists!");
        sInstance = this;
        m_appName = appName;
        m_height = height;
        m_width = width;
    }

    void Application::run(int argc, char **argv) {
        m_CommandLineArgs.clear();
        if (argc > 0 && argv) {
            m_CommandLineArgs.reserve(static_cast<size_t>(argc));
            for (int i = 0; i < argc; ++i)
                m_CommandLineArgs.emplace_back(argv[i]);
        }

        init();
        mainLoop();
        shutdown();
    }

    void Application::run() { run(0, nullptr); }

    void Application::init() {
        AXIOM_PROFILE_SCOPE("Application::Init");
        Window::Props props;
        props.height = m_height;
        props.width = m_width;
        props.title = m_appName;
        m_window = std::make_shared<Window>(props, m_eventBus);

        std::shared_ptr<ImGuiPanelLogsink> logsink =
            std::make_shared<ImGuiPanelLogsink>();

        addImGuiPanel(std::make_shared<LogPanel>(logsink));

#ifdef AXIOM_ENABLE_CONSOLE_LOG
        Logger::get().addSink(std::make_shared<ConsoleSink>());
#endif
        Logger::get().addSink(logsink);

        m_eventBus.subscribe<WindowCloseEvent>(
            [this](WindowCloseEvent &e) { return onWindowClose(e); });

        m_eventBus.subscribe<WindowResizeEvent>(
            [this](WindowResizeEvent &e) { return onWindowResize(e); });

        m_input.init(m_window->getNativeHandle());
        m_inputSystem.init();

        VFS::init();
        bool engineAssetPathUsedFallback = false;
        const std::string engineAssetPath = VFS::resolvePhysicalMountPath(
            AXIOM_ENGINE_ASSET_PATH,
            {"./axiom/assets", "../axiom/assets", "../../axiom/assets",
             "../../../axiom/assets", "../../../../axiom/assets"},
            m_CommandLineArgs, engineAssetPathUsedFallback);

        if (engineAssetPathUsedFallback)
            AXIOM_WARN("Engine asset path fallback is used: {}",
                       engineAssetPath);

        if (!VFS::mountPath("engine://", engineAssetPath))
            AXIOM_WARN("Failed to mount engine asset path: {}",
                       engineAssetPath);

        {
            auto windowSurfaceDesc = renderer::adapters::makeSDL3WindowSurfaceDesc(
                m_window->getNativeHandle());
            m_renderer = std::make_unique<renderer::Renderer>();
            if (auto result = m_renderer->init(windowSurfaceDesc); !result) {
                AXIOM_FATAL("Renderer-Initialisierung fehlgeschlagen");
                m_running = false;
                return;
            }

            renderer::View mainView;
            mainView.target.kind = renderer::RenderTargetKind::swapchain;
            mainView.target.surface = m_renderer->mainSurface();
            mainView.viewport = renderer::Viewport{0, 0, m_width, m_height};
            mainView.priority = 0;
            m_mainViewId = m_renderer->registerView(mainView);

            // ImGui registriert seinen eigenen Pass (cleart per loadOp).
            m_ImGuiLayer = IImGuiLayer::create(*m_window, *m_renderer);
            if (m_ImGuiLayer) {
                m_window->setNativeEventHook(
                    [layer = m_ImGuiLayer.get()](const SDL_Event &event) {
                        return layer->processEvent(event);
                    });
            } else {
                // Platzhalter-Pass, bis echte 2D-Passes existieren (Phase 6) -
                // zeigt zumindest, dass die komplette Pipeline (Acquire ->
                // Clear -> Present) tatsaechlich funktioniert.
                m_renderer->addPass(
                    std::make_unique<renderer::passes::ClearScreenPass>());
            }
        }

        onInit();
    }

    void Application::mainLoop() {
        double printTimer = 0.0f;

        while (m_running) {
            Time::update();
            axiom::profiling::Profiler::beginFrame();

            {
                AXIOM_PROFILE_FRAME();

                mainUpdate();
                double alpha = (Time::getTime() - mLastFixedUpdate) /
                               mFixedUpdateInterval;
                // ImGui::Render() (nur CPU) MUSS vor renderFrame() laufen:
                // der ImGuiPass liest die DrawData waehrend execute().
                imGuiRender();
                render(alpha);

                // Sekundaere ImGui-Fenster (ausgedockt) erst NACH dem
                // Praesentieren des Hauptfensters - siehe RenderAdditionalViewports().
                if (m_ImGuiLayer)
                    m_ImGuiLayer->renderAdditionalViewports();
            }

            axiom::profiling::Profiler::endFrame();

            printTimer += Time::getDeltaTime();

            if (printTimer > 1.0f) {
                axiom::profiling::Profiler::printLastFrame();
                printTimer = 0.0f;
            }
        }
    }

    void Application::render(double alpha) {
        AXIOM_PROFILE_SCOPE("Render");
        if (!m_renderer) {
            return;
        }

        m_renderer->beginFrame();

        onRender(alpha);

        m_systemManager.beginRenderFrame();
        for (auto &scene : m_Scenes) {
            if (scene) {
                m_systemManager.render(*scene, alpha);
            }
        }

        for (auto &layer : m_layerStack) {
            AXIOM_PROFILE_SCOPE(layer->getName());
            layer->onRender(alpha);
        }

        if (auto result = m_renderer->renderFrame(); !result) {
            AXIOM_ERROR("renderFrame() fehlgeschlagen");
        }
    }

    void Application::imGuiRender() {
        if (!m_ImGuiLayer)
            return;

        AXIOM_PROFILE_SCOPE("ImGuiRender");
        m_ImGuiLayer->begin();
        onImGuiRender();
        m_imGuiPanelManager.imGuiRender();
        for (auto &layer : m_layerStack) {
            AXIOM_PROFILE_SCOPE(layer->getName());
            layer->onImGuiRender();
        }
        m_ImGuiLayer->end();
    }

    void Application::preUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("PreUpdate");

        for (auto &scene : m_Scenes) {
            if (scene)
                m_systemManager.preUpdate(*scene, dt);
        }
        onPreUpdate(dt);
        for (auto &layer : m_layerStack) {
            AXIOM_PROFILE_SCOPE(layer->getName());
            layer->onPreUpdate(dt);
        }
    }

    void Application::postUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("PostUpdate");
        onPostUpdate(dt);
        for (auto &layer : m_layerStack) {
            AXIOM_PROFILE_SCOPE(layer->getName());
            layer->onPostUpdate(dt);
        }
        for (auto &scene : m_Scenes) {
            if (scene)
                m_systemManager.postUpdate(*scene, dt);
        }
    }

    void Application::fixedUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("FixedUpdate");
        for (auto &scene : m_Scenes) {
            if (scene)
                m_systemManager.fixedUpdate(*scene, dt);
        }
        onFixedUpdate(dt);
        for (auto &layer : m_layerStack) {
            AXIOM_PROFILE_SCOPE(layer->getName());
            layer->onFixedUpdate(dt);
        }
    }

    void Application::update(double dt) {
        AXIOM_PROFILE_SCOPE("Update");
        onUpdate(dt);
        for (auto &layer : m_layerStack) {
            AXIOM_PROFILE_SCOPE(layer->getName());
            layer->onUpdate(dt);
        }
        for (auto &scene : m_Scenes) {
            if (scene)
                m_systemManager.update(*scene, dt);
        }
        m_imGuiPanelManager.update(dt);
    }

    void Application::mainUpdate() {
        AXIOM_PROFILE_SCOPE("Application::MainUpdate");

        m_window->pollEvents();
        m_eventBus.dispatchQueued();

        m_inputSystem.update();

        double dt = Time::getDeltaTime();

        preUpdate(dt);
        update(dt);

        int maxSteps = 5;
        int steps = 0;

        while (Time::getTime() - mLastFixedUpdate >= mFixedUpdateInterval &&
               steps < maxSteps) {
            fixedUpdate(mFixedUpdateInterval);
            mLastFixedUpdate += mFixedUpdateInterval;
            steps++;
        }

        postUpdate(dt);
    }

    void Application::shutdown() {
        AXIOM_PROFILE_SCOPE("Application::Shutdown");
        onShutdown();
        m_layerStack.shutdown();

        // ImGui haengt am Vulkan-Device: VOR dem Renderer abbauen. Als Member
        // (public, weit oben deklariert) wuerde es sonst erst NACH m_Renderer
        // zerstoert.
        m_window->setNativeEventHook(nullptr);
        m_ImGuiLayer.reset();

        VFS::shutdown();
    }

} // namespace axiom
