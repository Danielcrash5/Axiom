// ===== axiom/src/core/Application.cpp =====
#include "axiom/core/Application.h"
#include "axiom/platform/Window.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Time.h"
#include "axiom/profiling/Profiler.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/Renderer2D.h"
#include "axiom/assets/VFS.h"
#include "axiom/ImGui/Panels/LogPanel.h"

#include <filesystem>
#include <vector>

namespace axiom {

    namespace {
        bool PathExists(const std::filesystem::path& path) {
            std::error_code ec;
            return std::filesystem::exists(path, ec);
        }

        std::filesystem::path ResolveExecutableDirectory(const std::vector<std::string>& commandLineArgs) {
            namespace fs = std::filesystem;

            if (commandLineArgs.empty() || commandLineArgs[0].empty())
                return {};

            std::error_code ec;
            fs::path executablePath = commandLineArgs[0];
            if (executablePath.is_relative())
                executablePath = fs::absolute(executablePath, ec);

            if (ec || !executablePath.has_parent_path())
                return {};

            return executablePath.parent_path();
        }

        std::string ResolveAssetPath(
            const std::string& configuredPath,
            const std::vector<std::string>& fallbackCandidates,
            const std::vector<std::string>& commandLineArgs,
            bool& usedFallback
        ) {
            namespace fs = std::filesystem;
            usedFallback = false;

            if (!configuredPath.empty() && PathExists(configuredPath))
                return configuredPath;

            const fs::path executableDir = ResolveExecutableDirectory(commandLineArgs);
            if (!configuredPath.empty() && !executableDir.empty()) {
                fs::path configured = configuredPath;
                if (configured.is_relative()) {
                    fs::path executableRelativePath = executableDir / configured;
                    if (PathExists(executableRelativePath))
                        return executableRelativePath.string();
                }
            }

            for (const auto& candidate : fallbackCandidates) {
                if (PathExists(candidate)) {
                    usedFallback = true;
                    return candidate;
                }
            }

            return configuredPath;
        }
    }

    Application* Application::s_Instance = nullptr;

    static double m_LastFixedUpdate = 0.0f;
    static double m_FixedUpdateInterval = 1.0 / 75.0;

    Application::Application(std::string AppName, uint32_t width, uint32_t height) {
        AXIOM_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;
        m_AppName = AppName;
        m_Height = height;
        m_Width = width;
    }

    void Application::Run(int argc, char** argv) {
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

    void Application::Run() {
        Run(0, nullptr);
    }

    void Application::Init() {
        AXIOM_PROFILE_SCOPE("Application::Init");
        Window::Props props;
        props.height = m_Height;
        props.width = m_Width;
        props.title = m_AppName;
        m_Window = std::make_unique<Window>(props, m_EventBus);

		std::shared_ptr<ImGuiPanelLogsink> logsink = std::make_shared<ImGuiPanelLogsink>();

		AddImGuiPanel(std::make_shared<LogPanel>(logsink));

#ifdef AXIOM_ENABLE_CONSOLE_LOG
        Logger::Get().AddSink(std::make_shared<ConsoleSink>());
#endif
		Logger::Get().AddSink(logsink);


        m_EventBus.Subscribe<WindowCloseEvent>(
            [this](WindowCloseEvent& e) {
                return OnWindowClose(e);
            }
        );

        m_EventBus.Subscribe<WindowResizeEvent>(
            [this](WindowResizeEvent& e) {
                return OnWindowResize(e);
            }
        );

        m_Input.Init(m_Window->GetNativeHandle());
        m_InputSystem.Init();

        Renderer::Init();
        RenderCommand::SetViewport(0, 0, m_Width, m_Height);

        VFS::Init();
        bool engineAssetPathUsedFallback = false;
        const std::string engineAssetPath = ResolveAssetPath(
            AXIOM_ENGINE_ASSET_PATH,
            {
                "./axiom/assets",
                "../axiom/assets",
                "../../axiom/assets",
                "../../../axiom/assets",
                "../../../../axiom/assets"
            },
            m_CommandLineArgs,
            engineAssetPathUsedFallback
        );

        if (engineAssetPathUsedFallback)
            AXIOM_WARN("Engine asset path fallback is used: {}", engineAssetPath);

        VFS::MountPath("engine://", engineAssetPath);
        m_ImGuiLayer = IImGuiLayer::Create(m_Window);
        Renderer2D::Init();

        m_Scenes.push_back(std::make_unique<Scene>());

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
                double alpha = (Time::GetTime() - m_LastFixedUpdate) / m_FixedUpdateInterval;
                Render(alpha);
				ImGuiRender();

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

        // User render
        OnRender(alpha);

        // System render
        m_SystemManager.BeginRenderFrame();
        for (auto& scene : m_Scenes) {
            if (scene)
                m_SystemManager.Render(*scene, alpha);
        }

        // Layer rendering
        for (auto& layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnRender(alpha);
        }

    }

	void Application::ImGuiRender() {
		AXIOM_PROFILE_SCOPE("ImGuiRender");
		m_ImGuiLayer->Begin();
		OnImGuiRender();
		m_ImGuiPanelManager.ImGuiRender();
		for (auto& layer : m_LayerStack) {
			AXIOM_PROFILE_SCOPE(layer->GetName());
			layer->OnImGuiRender();
		}
		m_ImGuiLayer->End();
	}

    void Application::PreUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("PreUpdate");

        for (auto& scene : m_Scenes) {
            if (scene)
                m_SystemManager.PreUpdate(*scene, dt);
        }
        OnPreUpdate(dt);
        for (auto& layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnPreUpdate(dt);
        }
    }

    void Application::PostUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("PostUpdate");
        OnPostUpdate(dt);
        for (auto& layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnPostUpdate(dt);
        }
        for (auto& scene : m_Scenes) {
            if (scene)
                m_SystemManager.PostUpdate(*scene, dt);
        }
    }

    void Application::FixedUpdate(double dt) {
        AXIOM_PROFILE_SCOPE("FixedUpdate");
        for (auto& scene : m_Scenes) {
            if (scene)
                m_SystemManager.FixedUpdate(*scene, dt);
        }
        OnFixedUpdate(dt);
        for (auto& layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnFixedUpdate(dt);
        }
    }

    void Application::Update(double dt) {
        AXIOM_PROFILE_SCOPE("Update");
        OnUpdate(dt);
        for (auto& layer : m_LayerStack) {
            AXIOM_PROFILE_SCOPE(layer->GetName());
            layer->OnUpdate(dt);
        }
        for (auto& scene : m_Scenes) {
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

        while (Time::GetTime() - m_LastFixedUpdate >= m_FixedUpdateInterval && steps < maxSteps) {
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
        Renderer2D::Shutdown();
        VFS::Shutdown();
    }

} // namespace axiom
