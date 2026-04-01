#pragma once
#include "axiom/platform/Window.h"
#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"
#include "axiom/core/Layerstack.h"
#include "axiom/input/Input.h"
#include "axiom/input/InputSystem.h"
#include <memory>

namespace axiom {

	class Application {
	public:
		Application(std::string AppName = "Axiom Engine", uint32_t width = 1280, uint32_t height = 720);
		virtual ~Application() = default;

		void Run();

		void PushLayer(std::unique_ptr<Layer> layer) {
			m_LayerStack.PushLayer(std::move(layer), m_EventBus);
		}

		void PushOverlay(std::unique_ptr<Layer> overlay) {
			m_LayerStack.PushOverlay(std::move(overlay), m_EventBus);
		}

		void PopLayer(Layer* layer) {
			m_LayerStack.PopLayer(layer);
		}

		void PopOverlay(Layer* overlay) {
			m_LayerStack.PopOverlay(overlay);
		}

		static Application& Get() {
			return *s_Instance;
		}

		EventBus& GetEventBus() {
			return m_EventBus;
		}

		std::unique_ptr<Window>& GetWindow() {
			return m_Window;
		}

		uint32_t GetWidth() const {
			return m_Width;
		}

		uint32_t GetHeigth() const {
			return m_Height;
		}

		Input& GetInput() {
			return m_Input;
		}

		InputSystem& GetInputSystem() {
			return m_InputSystem;
		}

	protected:

		// Virtual Gamehooks - Override these in your application subclass to implement game-specific behavior
		virtual void OnInit() {}
		virtual void OnShutdown() {}

		virtual void OnPreUpdate(double dt) {}
		virtual void OnPostUpdate(double dt) {}
		virtual void OnFixedUpdate(double dt) {}
		virtual void OnUpdate(double dt) {}

		virtual void OnRender(double alpha) {}

		void Close() {
			m_Running = false;
		}

	private:
		void MainLoop();
		void Init();
		void Shutdown();

		void MainUpdate();
		void PreUpdate(double dt);
		void PostUpdate(double dt);
		void FixedUpdate(double dt);
		void Update(double dt);

		void Render(double alpha);

		bool OnWindowClose(WindowCloseEvent& e) {
			Close();
			return true; // Return true to indicate that the event has been handled and should not be propagated further
		}

		bool OnWindowResize(WindowResizeEvent& e) {
			m_Width = e.width;
			m_Height = e.height;
			return false; // Return false to allow other listeners to handle the event as well
		}

	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
		std::string m_AppName;

		uint32_t m_Width, m_Height;

		static Application* s_Instance;

		InputSystem m_InputSystem;
		Input m_Input;

		LayerStack m_LayerStack;
		EventBus m_EventBus;

	};

}

#define GetMainEventBus() axiom::Application::Get().GetEventBus() 
#define GetMainWindow() axiom::Application::Get().GetWindow()
#define GetMainInput() axiom::Application::Get().GetInput()
#define GetMainInputSystem() axiom::Application::Get().GetInputSystem()
