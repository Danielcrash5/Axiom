#include "axiom/core/Application.h"
#include "axiom/platform/Window.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Time.h"
#include "axiom/profiling/Profiler.h"
#include "axiom/renderer/Renderer.h"

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
		AXIOM_PROFILE_SCOPE("Application::Init");
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

		m_Input.Init(m_Window->GetNativeHandle());
		m_InputSystem.Init();

		Renderer::Init();

		OnInit();
	}

	void Application::MainLoop() {

		float printTimer = 0.0f;

		while (m_Running) {

			axiom::profiling::Profiler::BeginFrame();

			{
				AXIOM_PROFILE_FRAME();

				MainUpdate();
				Render();
			}

			axiom::profiling::Profiler::EndFrame();

			printTimer += Time::GetDeltaTime();

			if (printTimer > 1.0f) {
				axiom::profiling::Profiler::PrintLastFrame();
				printTimer = 0.0f;
			}
		}
	}

	void Application::Render() {
		AXIOM_PROFILE_SCOPE("Render");

		Renderer::BeginFrame();
		Renderer::Clear();

		OnRender();

		for (auto& layer : m_LayerStack) {
			AXIOM_PROFILE_SCOPE(layer->GetName());
			layer->OnRender();
		}

		Renderer::EndFrame();

		m_Window->SwapBuffers();
	}

	void Application::PreUpdate(float dt) {
		AXIOM_PROFILE_SCOPE("PreUpdate");

		OnPreUpdate(dt);
		for (auto& layer : m_LayerStack) {
			AXIOM_PROFILE_SCOPE(layer->GetName());
			layer->OnPreUpdate(dt);
		}
	}

	void Application::PostUpdate(float dt) {
		AXIOM_PROFILE_SCOPE("PostUpdate");
		OnPostUpdate(dt);
		for (auto& layer : m_LayerStack) {
			AXIOM_PROFILE_SCOPE(layer->GetName());
			layer->OnPostUpdate(dt);
		}
	}

	void Application::FixedUpdate(float dt) {
		AXIOM_PROFILE_SCOPE("FixedUpdate");
		OnFixedUpdate(dt);
		for (auto& layer : m_LayerStack) {
			AXIOM_PROFILE_SCOPE(layer->GetName());
			layer->OnFixedUpdate(dt);
		}
	}

	void Application::Update(float dt) {
		AXIOM_PROFILE_SCOPE("Update");
		OnUpdate(dt);
		for (auto& layer : m_LayerStack) {
			AXIOM_PROFILE_SCOPE(layer->GetName());
			layer->OnUpdate(dt);
		}
	}

	static float m_LastFixedUpdate = 0.0f;
	static float m_FixedUpdateInterval = 1.0f / 60.0f; // 60 FPS

	void Application::MainUpdate() {

		AXIOM_PROFILE_SCOPE("Application::MainUpdate");

		float dt = Time::GetDeltaTime();

		PreUpdate(dt);
		Update(dt);

		while (Time::GetTime() - m_LastFixedUpdate >= m_FixedUpdateInterval) {
			FixedUpdate(m_FixedUpdateInterval);
			m_LastFixedUpdate += m_FixedUpdateInterval;
		}

		PostUpdate(dt);

		m_Input.Update();
		m_InputSystem.Update();

		m_Window->PollEvents();
		m_EventBus.DispatchQueued();
	}

	void Application::Shutdown() {
		AXIOM_PROFILE_SCOPE("Application::Shutdown");
		OnShutdown();
		m_LayerStack.Shutdown();
	}
}