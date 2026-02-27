#include "axiom/core/Application.h"
#include "axiom/platform/Window.h"
#include <chrono>

namespace axiom {

    void Application::Run() {
        Init();
        MainLoop();
        OnShutdown();
    }

    void Application::Init() {
		Window::Props props;
		props.title = "Axiom Engine";
        m_Window = std::make_unique<Window>(props);
		OnInit();
	}

    void Application::MainLoop() {
        using clock = std::chrono::high_resolution_clock;

        auto lastTime = clock::now();

        while (m_Running) {
            auto now = clock::now();
            float dt =
                std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;
			m_Window->PollEvents();

            OnUpdate(dt);

			if (m_Window->ShouldClose())
				Close();
        }
    }

}