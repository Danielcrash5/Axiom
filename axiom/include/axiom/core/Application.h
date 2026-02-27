#pragma once
#include "axiom/platform/Window.h"
#include <memory>

namespace axiom {

    class Application {
    public:
        Application() = default;
        virtual ~Application() = default;

        void Run();

    protected:
        virtual void OnInit() {
        }
        virtual void OnUpdate(float dt) {
        }
        virtual void OnShutdown() {
        }

        void Close() {
            m_Running = false;
        }

        std::unique_ptr<Window>& GetWindow() {
            return m_Window;
		}

    private:
        void MainLoop();
        void Init();

    private:
        bool m_Running = true;
        std::unique_ptr<Window> m_Window;
    };

}