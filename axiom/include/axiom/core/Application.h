// ===== axiom/include/axiom/core/Application.h =====
#pragma once
#include "axiom/platform/Window.h"
#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"
#include "axiom/core/Layerstack.h"
#include "axiom/input/Input.h"
#include "axiom/input/InputSystem.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/ecs/Scene.h"
#include "axiom/ecs/SystemManager.h"
#include "axiom/ImGui/IImGuiLayer.h"
#include "axiom/ImGui/ImGuiPanelManager.h"
#include <memory>
#include <string>
#include <vector>

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

		void AddImGuiPanel(const std::shared_ptr<IImGuiPanel>& panel) {
			m_ImGuiPanelManager.AddPanel(panel);
		}

		void RemoveImGuiPanel(const std::string& name) {
			m_ImGuiPanelManager.RemovePanel(name);
		}

        static Application& Get() {
            return *s_Instance;
        }

        void Run(int argc, char** argv);

        const std::vector<std::string>& GetCommandLineArgs() const {
            return m_CommandLineArgs;
        }

        EventBus& GetEventBus() {
            return m_EventBus;
        }

        std::shared_ptr<Window> GetWindow() {
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

        Scene& GetScene() {
            return *m_Scenes.front();
        }

        Scene& CreateScene() {
            m_Scenes.push_back(std::make_unique<Scene>());
            return *m_Scenes.back();
        }

        std::vector<std::unique_ptr<Scene>>& GetScenes() {
            return m_Scenes;
        }

        template<typename T, typename... Args>
        std::shared_ptr<T> RegisterSystem(Args&&... args) {
            return m_SystemManager.AddSystem<T>(std::forward<Args>(args)...);
        }

        SystemManager& GetSystemManager() {
            return m_SystemManager;
        }

        const SystemManager& GetSystemManager() const {
            return m_SystemManager;
        }

	

    protected:
        virtual void OnInit() {}
        virtual void OnShutdown() {}

        virtual void OnPreUpdate(double dt) {}
        virtual void OnPostUpdate(double dt) {}
        virtual void OnFixedUpdate(double dt) {}
        virtual void OnUpdate(double dt) {}

        virtual void OnRender(double alpha) {}
		virtual void OnImGuiRender() {}

        void Close() {
            m_Running = false;
        }

		std::unique_ptr<IImGuiLayer> m_ImGuiLayer;

        std::vector<std::unique_ptr<Scene>> m_Scenes;
        std::vector<std::string> m_CommandLineArgs;

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
		void ImGuiRender();

        bool OnWindowClose(WindowCloseEvent& e) {
            Close();
            return true;
        }

        bool OnWindowResize(WindowResizeEvent& e) {
            m_Width = e.width;
            m_Height = e.height;
            m_Renderer->on_window_resize(m_Width, m_Height);
            m_SystemManager.OnViewportResize(e.width, e.height);
            return false;
        }
    private:
        bool m_Running = true;
        std::shared_ptr<Window> m_Window;
        std::string m_AppName;

        std::unique_ptr<Renderer> m_Renderer;

        uint32_t m_Width, m_Height;

        static Application* s_Instance;

        InputSystem m_InputSystem;
        Input m_Input;

		ImGuiPanelManager m_ImGuiPanelManager;

        LayerStack m_LayerStack;
        EventBus m_EventBus;

		

        SystemManager m_SystemManager;
    };

}

#define GetMainEventBus() axiom::Application::Get().GetEventBus() 
#define GetMainWindow() axiom::Application::Get().GetWindow()
#define GetMainInput() axiom::Application::Get().GetInput()
#define GetMainInputSystem() axiom::Application::Get().GetInputSystem()
