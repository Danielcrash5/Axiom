#include "axiom/core/Application.h"
#include "axiom/platform/Window.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Time.h"
#include "axiom/profiling/Profiler.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/Renderer2D.h"

namespace axiom {

	Application* Application::s_Instance = nullptr;

// Fixed update timing state (file scope so all functions can use them)
static double m_LastFixedUpdate = 0.0f;
static double m_FixedUpdateInterval = 1.0 / 75.0; // 75 FPS

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
	// Initialize renderer
	axiom::Renderer::Init();
	axiom::Renderer2D::Init();

	// setup a simple orthographic camera for 2D
	m_Camera.setOrthographic(0.0f, (float)m_Width, 0.0f, (float)m_Height, -1.0f, 1.0f);

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

	// clear and begin frame
	axiom::Renderer::Begin({ 0.1f, 0.1f, 0.1f, 1.0f });

	// 2D Renderer frame
	axiom::Renderer2D::Begin(m_Camera);

	OnRender(alpha);

	for (auto& layer : m_LayerStack) {
		AXIOM_PROFILE_SCOPE(layer->GetName());
		layer->OnRender(alpha);
	}

	axiom::Renderer2D::End();

	axiom::Renderer::End();

	m_Window->SwapBuffers();
	}

	void Application::PreUpdate(double dt) {
		AXIOM_PROFILE_SCOPE("PreUpdate");

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
	}

	void Application::FixedUpdate(double dt) {
		AXIOM_PROFILE_SCOPE("FixedUpdate");
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
	}

	void Application::MainUpdate() {

		AXIOM_PROFILE_SCOPE("Application::MainUpdate");

		m_Window->PollEvents();
		m_EventBus.DispatchQueued();

		m_InputSystem.Update();

		double dt = Time::GetDeltaTime();

		PreUpdate(dt);
		Update(dt);

		// -------- Fixed Update --------
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

	axiom::Renderer2D::Shutdown();
	axiom::Renderer::Shutdown();
	}
}