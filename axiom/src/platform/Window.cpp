#include "axiom/platform/Window.h"
#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"
#include "axiom/core/Logger.h"
#include <format>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace axiom {

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