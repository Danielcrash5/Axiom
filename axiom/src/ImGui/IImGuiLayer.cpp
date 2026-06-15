#include "axiom/ImGui/IImGuiLayer.h"
#include "axiom/platform/Window.h"
#include "axiom/core/Logger.h"
#include <memory>

namespace axiom {
	std::unique_ptr<IImGuiLayer> IImGuiLayer::Create(std::unique_ptr<Window>& window) {
		return nullptr;
	}
}