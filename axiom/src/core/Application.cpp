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

		m_Input.Init(m_Window->GetNativeHandle());
		m_InputSystem.Init();

		OnInit();
	}

	void Application::MainLoop() {

		while (m_Running) {
			float dt = Time::GetDeltaTime();

			OnUpdate(dt);

			for (auto& layer : m_LayerStack)
				layer->OnUpdate(dt);

			m_Input.Update();
			m_InputSystem.Update();

			m_Window->PollEvents();
			m_EventBus.DispatchQueued();
		}
	}

	void Application::Shutdown() {
		OnShutdown();
		m_LayerStack.Shutdown();
	}
}