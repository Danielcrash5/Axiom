#include "axiom/platform/opengl/OpenGLContext.h"
#include "axiom/core/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace axiom {

    static void GLDebugCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar* message, const void* userParam) {
        AXIOM_ERROR("OpenGL: {}", message);
    }

    OpenGLContext::OpenGLContext(GLFWwindow* window)
        : m_WindowHandle(window) {
    }

    void OpenGLContext::Init() {
        glfwMakeContextCurrent(m_WindowHandle);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            AXIOM_ERROR("Failed to initialize GLAD");
            return;
        }

        AXIOM_INFO("OpenGL Info:");
        AXIOM_INFO("  Vendor: {}", (const char*)glGetString(GL_VENDOR));
        AXIOM_INFO("  Renderer: {}", (const char*)glGetString(GL_RENDERER));
        AXIOM_INFO("  Version: {}", (const char*)glGetString(GL_VERSION));

        // Debug Output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageControl(
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DEBUG_SEVERITY_NOTIFICATION,
            0,
            nullptr,
            GL_FALSE
        );

        glDebugMessageCallback(GLDebugCallback, nullptr);
    }

    void OpenGLContext::SwapBuffers() {
        glfwSwapBuffers(m_WindowHandle);
    }

}