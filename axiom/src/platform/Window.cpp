#include "axiom/platform/Window.h"
#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"
#include "axiom/core/Logger.h"
#include <format>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace axiom {

#include <format> // C++20
#include <string>

	void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id,
								GLenum severity, GLsizei length,
								const GLchar* message, const void* userParam) {
		// Optional: bekannte Spam-IDs ignorieren
		switch (id) {
		case 131169:
		case 131185:
		case 131218:
		case 131204:
			return;
		}

		std::string sourceStr;
		switch (source) {
		case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "WindowSystem"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "ShaderCompiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "ThirdParty"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
		}

		std::string typeStr;
		switch (type) {
		case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "UndefinedBehavior"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "PushGroup"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "PopGroup"; break;
		case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
		}

		// Mit std::format eine einzelne Nachricht bauen
		std::string logMsg = std::format("[OpenGL][{}][{}][ID:{}] {}",
										 sourceStr, typeStr, id, message);

		// Severity Mapping zu deinen Makros
		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			AXIOM_ERROR("{}", logMsg);
			break;

		case GL_DEBUG_SEVERITY_MEDIUM:
			AXIOM_WARN("{}", logMsg);
			break;

		case GL_DEBUG_SEVERITY_LOW:
			AXIOM_INFO("{}", logMsg);
			break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
			AXIOM_DEBUG("{}", logMsg);
			break;
		}
	}

	static bool s_GLFWInitialized = false;

	Window::Window(const Props& props, EventBus& eventBus) :
		m_EventBus(eventBus) {

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

		glfwSetErrorCallback([](int error, const char* description) {
			AXIOM_ERROR("GLFW Error ({}): {}", error, description);
		});

		// Korrekte Kontext-Hints verwenden
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

		// Fenster erstellen
		m_Window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
		if (!m_Window) {
			// glfwGetError returns an error code and optionally a description via the
			// out-parameter. Call it correctly to obtain the description string.
			const char* desc = nullptr;
			int err = glfwGetError(&desc);
			(void)err; // err may be unused - keep for debugging if needed
			throw std::runtime_error(std::string("Failed to create GLFW window: ") + (desc ? desc : "unknown"));
		}

		// Kontext aktiv machen bevor GL-Funktionen geladen werden
		glfwMakeContextCurrent(m_Window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			AXIOM_ERROR("Failed to initialize GLAD");
			return;
		}

		// Debug Output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);

		// Optional: weniger Spam
		glDebugMessageControl(
			GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DEBUG_SEVERITY_NOTIFICATION,
			0, nullptr,
			GL_FALSE
		);

		glfwSetWindowUserPointer(m_Window, this);

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			CharEvent event{ codepoint };
			win->m_EventBus.Publish(event);
							});

		glfwSetCharModsCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint, int mods) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			CharModsEvent event{ codepoint, mods };
			win->m_EventBus.Publish(event);
								});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double x, double y) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			MouseMoveEvent event{ x, y };
			win->m_EventBus.Publish(event);
								 });

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			WindowResizeEvent event{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
			win->m_Width = width;
			win->m_Height = height;
			win->m_EventBus.Publish(event);
								  });

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			WindowCloseEvent event;
			win->m_EventBus.Publish(event);
								   });

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			KeyEvent event{ key, scancode, action, mods };
			win->m_EventBus.Publish(event);
						   });

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			MouseScrollEvent event{ xOffset, yOffset };
			win->m_EventBus.Publish(event);
							  });

		glfwSetDropCallback(m_Window, [](GLFWwindow* window, int count, const char** paths) {
			auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			FileDropEvent event{ count, paths };
			win->m_EventBus.Publish(event);
							});

		m_Vsync = props.vsync;


		if (m_Vsync) {
			glfwSwapInterval(1);
		} else {
			glfwSwapInterval(0);
		}

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

	void Window::SwapBuffers() {
		glfwSwapBuffers(m_Window);
	}

	void Window::ToggleVsync() {
		m_Vsync = !m_Vsync;
		glfwSwapInterval(m_Vsync);
	}

}