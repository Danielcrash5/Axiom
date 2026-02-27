#include "axiom/platform/Window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace axiom {

    static bool s_GLFWInitialized = false;

    Window::Window(const Props& props) {
        Init(props);
    }

    Window::~Window() {
        Shutdown();
    }

    void Window::Init(const Props& props) {
        if (!s_GLFWInitialized) {
            if (!glfwInit())
                throw std::runtime_error("Failed to initialize GLFW");

            s_GLFWInitialized = true;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // wichtig für Vulkan

        m_Window = glfwCreateWindow(
            props.width,
            props.height,
            props.title.c_str(),
            nullptr,
            nullptr
        );

        if (!m_Window)
            throw std::runtime_error("Failed to create GLFW window");

        m_Width = props.width;
        m_Height = props.height;
    }

    void Window::Shutdown() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }

        if (s_GLFWInitialized) {
            glfwTerminate();
            s_GLFWInitialized = false;
        }
    }

    void Window::PollEvents() {
        glfwPollEvents();
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

    uint32_t Window::GetWidth() const {
        return m_Width;
    }

    uint32_t Window::GetHeight() const {
        return m_Height;
    }

    void* Window::GetNativeHandle() const {
        return m_Window;
    }

}