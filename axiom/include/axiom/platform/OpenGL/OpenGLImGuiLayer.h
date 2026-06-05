#pragma once

#include <memory>
#include "axiom/ImGui/IImGuiLayer.h"
#include <SDL3/SDL.h>

namespace axiom {
	class OpenGLImGuiLayer : public IImGuiLayer {
	public:
		OpenGLImGuiLayer(std::unique_ptr<Window>& window);
		virtual ~OpenGLImGuiLayer();

		virtual void Begin() override;
		virtual void End() override;

	private:
		void RenderDockspace();
		int TranslateMouseButton(int axiomButton);

		SDL_Window* m_SDLWindow;
	};
}