// ===== axiom/src/core/Application.cpp =====
#include "axiom/core/Application.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Time.h"
#include "axiom/platform/Window.h"
#include "axiom/profiling/Profiler.h"
// #include "axiom/renderer/Renderer.h"
// #include <axiom/renderer/RendererRHI.h>
#include "axiom/ImGui/Panels/LogPanel.h"
#include "axiom/assets/VFS.h"
#include <vector>

namespace axiom {

    Application *Application::s_Instance = nullptr;

    static double m_LastFixedUpdate = 0.0f;
    static double m_FixedUpdateInterval = 1.0 / 75.0;

    Application::Application(std::string AppName, uint32_t width,
                             uint32_t height) {
        AXIOM_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;
        m_AppName = AppName;
        m_Height = height;
        m_Width = width;
    }

    void Application::Run(int argc, char **argv) {
        m_CommandLineArgs.clear();
        if (argc > 0 && argv) {
            m_CommandLineArgs.reserve(static_cast<size_t>(argc));
            for (int i = 0; i < argc; ++i)
                m_CommandLineArgs.emplace_back(argv[i]);
        }

        Init();
        MainLoop();
        Shutdown();
    }

    void Application::Run() { Run(0, nullptr); }

    void Application::Init() {
        AXIOM_PROFILE_SCOPE("Application::Init");
        Window::Props props;
        props.height = m_Height;
        props.width = m_Width;
        props.title = m_AppName;
        m_Window = std::make_shared<Window>(props, m_EventBus);

        std::shared_ptr<ImGuiPanelLogsink> logsink =
            std::make_shared<ImGuiPanelLogsink>();

        // AddImGuiPanel(std::make_shared<LogPanel>(logsink));

#ifdef AXIOM_ENABLE_CONSOLE_LOG
        Logger::Get().AddSink(std::make_shared<ConsoleSink>());
#endif
        Logger::Get().AddSink(logsink);

        m_EventBus.Subscribe<WindowCloseEvent>(
            [this](WindowCloseEvent &e) { return OnWindowClose(e); });

        m_EventBus.Subscribe<WindowResizeEvent>(
            [this](WindowResizeEvent &e) { return OnWindowResize(e); });

        m_Input.Init(m_Window->GetNativeHandle());
        m_InputSystem.Init();

        VFS::Init();
        bool engineAssetPathUsedFallback = false;
        const std::string engineAssetPath = VFS::ResolvePhysicalMountPath(
            AXIOM_ENGINE_ASSET_PATH,
            {"./axiom/assets", "../axiom/assets", "../../axiom/assets",
             "../../../axiom/assets", "../../../../axiom/assets"},
            m_CommandLineArgs, engineAssetPathUsedFallback);

        if (engineAssetPathUsedFallback)
            AXIOM_WARN("Engine asset path fallback is used: {}",
                       engineAssetPath);

        if (!VFS::MountPath("engine://", engineAssetPath))
            AXIOM_WARN("Failed to mount engine asset path: {}",
                       engineAssetPath);

        {
            auto windowSurfaceDesc = renderer::adapters::makeSDL3WindowSurfaceDesc(
                m_Window->GetNativeHandle());
            m_Renderer = std::make_unique<renderer::Renderer>();
            if (auto result = m_Renderer->init(windowSurfaceDesc); !result) {
                AXIOM_FATAL("Renderer-Initialisierung fehlgeschlagen");
                m_Running = false;
                return;
            }

            renderer::View mainView;
            mainView.target.kind = renderer::RenderTargetKind::Swapchain;
            mainView.target.surface = m_Renderer->mainSurface();
            mainView.viewport = renderer::Viewport{0, 0, m_Width, m_Height};
            mainView.priority = 0;
            m_MainViewId = m_Renderer->registerView(mainView);

            // ImGui registriert seinen eigenen Pass (cleart per loadOp).
            m_ImGuiLayer = IImGuiLayer::Create(*m_Window, *m_Renderer);
            if (m_ImGuiLayer) {
                m_Window->SetNativeEventHook(
                    [layer = m_ImGuiLayer.get()](const SDL_Event &event) {
                        return layer->ProcessEvent(event);
                    });
            } else {
                // Platzhalter-Pass, bis echte 2D-Passes existieren (Phase 6) -
                // zeigt zumindest, dass die komplette Pipeline (Acquire ->
                // Clear -> Present) tatsaechlich funktioniert.
                m_Renderer->addPass(
                    std::make_unique<renderer::passes::ClearScreenPass>());
            }
        }

        OnInit();
    }

    void Application::MainLoop() {
        double printTimer = 0.0f;

        while (m_Running) {
            Time::Update();
            axiom::profiling::Profiler::BeginFrame();

            {
                AXIOM_PROFILE_FRAME();

                MainUpdate();
                double alpha = (Time::GetTime() - m_LastFixedUpdate) /
                               m_FixedUpdateInterval;
                // ImGui::Render() (nur CPU) MUSS vor renderFrame() laufen:
                // der ImGuiPass liest die DrawData waehrend execute().
                ImGuiRender();
                Render(alpha);

                m_Window->SwapBuffers();
            }

            axiom::profiling::Profiler::EndFrame();

            printTimer += Time::GetDeltaTime();

            if (printTimer > 1.0f) {
                axiom::profiling::Profiler::PrintLastFrame();
                printTimer = 0.0f;
            }
        }
    }

    void Application::Render(double alpha) {
        AXIOM_PROFILE_SCOPE("Render");
        if (!m_Renderer) {
            return;
        }

        m_Renderer->beginFrame();

        OnRender(alpha);

        m_SystemManager.BeginRenderFrame();
        for (auto &scene : m_Scenes) {
            if (scene) {
                m_SystemManager.Render(*scene, alpha);
            }
        }

        for (auto &layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnRender(alpha);
        }

        if (auto result = m_Renderer->renderFrame(); !result) {
            AXIOM_ERROR("renderFrame() fehlgeschlagen");
        }
    }

    void Application::ImGuiRender() {
        if (!m_ImGuiLayer)
            return;

        AXIOM_PROFILE_SCOPE("ImGuiRender");
        m_ImGuiLayer->Begin();
        OnImGuiRender();
        m_ImGuiPanelManager.ImGuiRender();
        for (auto &layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnImGuiRender();
        }
        m_ImGuiLayer->End();
    }

    void Application::PreUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("PreUpdate");

        for (auto &scene : m_Scenes) {
            if (scene)
                m_SystemManager.PreUpdate(*scene, dt);
        }
        OnPreUpdate(dt);
        for (auto &layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnPreUpdate(dt);
        }
    }

    void Application::PostUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("PostUpdate");
        OnPostUpdate(dt);
        for (auto &layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnPostUpdate(dt);
        }
        for (auto &scene : m_Scenes) {
            if (scene)
                m_SystemManager.PostUpdate(*scene, dt);
        }
    }

    void Application::FixedUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("FixedUpdate");
        for (auto &scene : m_Scenes) {
            if (scene)
                m_SystemManager.FixedUpdate(*scene, dt);
        }
        OnFixedUpdate(dt);
        for (auto &layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnFixedUpdate(dt);
        }
    }

    void Application::Update(double dt) {
        AXIOM_PROFILE_SCOPE("Update");
        OnUpdate(dt);
        for (auto &layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnUpdate(dt);
        }
        for (auto &scene : m_Scenes) {
            if (scene)
                m_SystemManager.Update(*scene, dt);
        }
        m_ImGuiPanelManager.Update(dt);
    }

    void Application::MainUpdate() {
        AXIOM_PROFILE_SCOPE("Application::MainUpdate");

        m_Window->PollEvents();
        m_EventBus.DispatchQueued();

        m_InputSystem.Update();

        double dt = Time::GetDeltaTime();

        PreUpdate(dt);
        Update(dt);

        int maxSteps = 5;
        int steps = 0;

        while (Time::GetTime() - m_LastFixedUpdate >= m_FixedUpdateInterval &&
               steps < maxSteps) {
            FixedUpdate(m_FixedUpdateInterval);
            m_LastFixedUpdate += m_FixedUpdateInterval;
            steps++;
        }

        PostUpdate(dt);
    }

    void Application::Shutdown() {
        AXIOM_PROFILE_SCOPE("Application::Shutdown");
        OnShutdown();
        m_LayerStack.Shutdown();

        // ImGui haengt am Vulkan-Device: VOR dem Renderer abbauen. Als Member
        // (public, weit oben deklariert) wuerde es sonst erst NACH m_Renderer
        // zerstoert.
        m_Window->SetNativeEventHook(nullptr);
        m_ImGuiLayer.reset();

        VFS::Shutdown();
    }

} // namespace axiom
