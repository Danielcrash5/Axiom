#pragma once

#include <string>

struct GLFWwindow;

namespace axiom {

    class Window {
    public:
        struct Props {
            uint32_t width = 1280;
            uint32_t height = 720;
            std::string title = "Engine";
        };

    public:
        explicit Window(const Props& props);
        ~Window();

        void PollEvents();

        bool ShouldClose() const;

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        void* GetNativeHandle() const;

    private:
        void Init(const Props& props);
        void Shutdown();

    private:
        GLFWwindow* m_Window = nullptr;

        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

}