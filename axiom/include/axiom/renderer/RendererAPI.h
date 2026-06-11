#pragma once


namespace axiom {
	enum class RendererAPI {
		Vulkan,
		OpenGL,
		WebGPU,
		None    //z.B. für Headless server built oder keine unterstützte API gefunden
	};
}