#include "axiom/core/Application.h"
#include "axiom/platform/Window.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Time.h"

namespace axiom {

	Application* Application::s_Instance = nullptr;

	Application::Application(std::string AppName, uint32_t width, uint32_t height) {
		AXIOM_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_AppName = AppName;
		m_Height = height;
		m_Width = width;
		
	}


	void Application::Run() {
		Init();
		MainLoop();
		Shutdown();
	}

	void Application::Init() {
		Window::Props props;
		props.height = m_Height;
		props.width = m_Width;
		props.title = m_AppName;
		m_Window = std::make_unique<Window>(props, m_EventBus);

#ifdef AXIOM_ENABLE_CONSOLE_LOG
		Logger::Get().AddSink(
			std::make_unique<ConsoleSink>()
		);
#endif
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

		m_Renderer = std::make_unique<Renderer>();

		m_Renderer->Init((GLFWwindow*)m_Window->GetNativeHandle());
		m_Input.Init(m_Window->GetNativeHandle());
		m_InputSystem.Init();


		OnInit();
	}

	void Application::MainLoop() {

		while (m_Running) {
			MainUpdate();

			Render();
		}
	}

	void Application::Render() {

		m_Renderer->BeginFrame();
		OnRender();
		for (auto& layer : m_LayerStack)
			layer->OnRender();

		m_Renderer->EndFrame();
	}

	void Application::PreUpdate(float dt) {
		OnPreUpdate(dt);
		for (auto& layer : m_LayerStack)
			layer->OnPreUpdate(dt);
	}

	void Application::PostUpdate(float dt) {
		OnPostUpdate(dt);
		for (auto& layer : m_LayerStack)
			layer->OnPostUpdate(dt);
	}

	void Application::FixedUpdate(float dt) {
		OnFixedUpdate(dt);
		for (auto& layer : m_LayerStack)
			layer->OnFixedUpdate(dt);
	}

	void Application::Update(float dt) {
		OnUpdate(dt);
		for (auto& layer : m_LayerStack)
			layer->OnUpdate(dt);
	}

	static float m_LastFixedUpdate = 0.0f;
	static float m_FixedUpdateInterval = 1.0f / 60.0f; // 60 FPS

	void Application::MainUpdate() {
		// Calculate delta time
		float dt = Time::GetDeltaTime();

		// Call update functions
		PreUpdate(dt);
		Update(dt);
		while (Time::GetTime() - m_LastFixedUpdate >= m_FixedUpdateInterval) {
			FixedUpdate(m_FixedUpdateInterval);
			m_LastFixedUpdate += m_FixedUpdateInterval;
		}
		PostUpdate(dt);

		Render();

		// Update input systems after all updates to ensure we have the latest input state for the next frame
		m_Input.Update();
		m_InputSystem.Update();

		// Process events
		m_Window->PollEvents();
		m_EventBus.DispatchQueued();
	}

	void Application::Shutdown() {
		OnShutdown();
		m_LayerStack.Shutdown();
	}
}