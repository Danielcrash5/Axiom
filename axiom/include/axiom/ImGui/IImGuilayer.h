#pragma once

#include <memory>
#include <imgui.h>
#include "axiom/platform/Window.h"
#include "axiom/core/Layer.h"
#include "axiom/events/Events.h"
#include "axiom/events/EventBus.h"

namespace axiom {

	enum class DockspaceType {
		None = 0,				// kein Dockspace, Fenster verhält sich normal
		Fullscreen = 1 << 0,	// Dockspace füllt das gesamte Fenster aus, mit Ränder oder Titelbar
		Passthrough = 1 << 1	// Dockspace ist durchsichtig, Engine kann normal in Fenster Rendern, ImGui-Elemente werden darüber gezeichnet
	};

	class IImGuiLayer : public Layer {
	public:
		IImGuiLayer() : Layer("ImGui"), m_DockspaceType(DockspaceType::None) {}
		virtual ~IImGuiLayer() = default;
		virtual void Begin() = 0;
		virtual void End() = 0;

		inline void SetDockspaceType(DockspaceType type) { m_DockspaceType = type; }

		static std::unique_ptr<IImGuiLayer> Create(std::unique_ptr<Window>& window);

		ImFont* GetDefaultFont() const { return m_DefaultFont; }
		ImFont* GetMonospaceFont() const { return m_MonospaceFont; }

	protected:
		DockspaceType m_DockspaceType;
		ImFont* m_DefaultFont = nullptr;
		ImFont* m_MonospaceFont = nullptr;
	};
}