#pragma once

struct GLFWwindow;

namespace axiom {

    class OpenGLContext {
    public:

        OpenGLContext(GLFWwindow* window);

        void Init();
        void SwapBuffers();
        void SetViewport(int width, int height);

    private:

        GLFWwindow* m_WindowHandle;

    };

}