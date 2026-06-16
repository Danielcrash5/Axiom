#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/RendererRHI.h> // Holt das korrekte compile-time Backend (GraphicsDevice)
#include <axiom/core/Logger.h>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <iostream>

namespace axiom {

	Renderer::Renderer(SDL_Window* window) {
		m_window = window;

		// 4. Das statische RHI-Grafikgerät initialisieren
		try {
			m_device = std::make_unique<GraphicsDevice>(m_window);
			AXIOM_INFO("Renderer-Subsystem erfolgreich initialisiert.");
		}
		catch (const std::exception& e) {
			throw e;
		}
	}

	Renderer::~Renderer() {
		// Explizite Freigabe-Reihenfolge: Erst GPU-Gerät, dann Fenster, dann SDL-Schnittstelle
		m_device.reset();
		AXIOM_INFO("Renderer-Subsystem sauber heruntergefahren.");
	}

	// In src/renderer/Renderer.cpp
	bool Renderer::begin_frame(CommandBuffer& outCmdBuffer) {
		return m_device->begin_frame(outCmdBuffer);
	}

	void Renderer::end_frame() {
		m_device->end_frame();
	}

	GraphicsDevice& Renderer::get_device() {
		return *m_device;
	}

	void Renderer::on_window_resize(int newWidth, int newHeight) {
		if (m_device) {
			m_device->handle_resize(newWidth, newHeight);
		}
	}


} // namespace Axiom