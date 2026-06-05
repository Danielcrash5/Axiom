#include "axiom/ImGui/IImGuiLayer.h"
#include "axiom/platform/Window.h"
#include "axiom/renderer/RendererAPI.h"
#include "axiom/platform/OpenGL/OpenGLImGuiLayer.h"
#include "axiom/core/Logger.h"
#include <memory>

namespace axiom {
	std::unique_ptr<IImGuiLayer> IImGuiLayer::Create(std::unique_ptr<Window>& window) {
		switch (RendererAPI::GetAPI()) {
		case RendererAPIType::None:
			AXIOM_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPIType::OpenGL:
			return std::make_unique<OpenGLImGuiLayer>(window);
		default:
			AXIOM_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}