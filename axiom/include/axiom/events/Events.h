#pragma once

namespace axiom {
	//------------------------------------------------
	//------------- Window Events --------------------
	//------------------------------------------------

	struct WindowCloseEvent {
		bool closed = true;
	};

	struct WindowResizeEvent {
		uint32_t width;
		uint32_t height;
	};

	//------------------------------------------------
	//------------- Input Events ---------------------
	//------------------------------------------------

	struct KeyEvent {
		int key;
		int scancode;
		int action;
		int mods;
		int repeat;
	};

	struct MouseButtonEvent {
		int button;
		int action;
		int mods;
	};

	struct MouseMoveEvent {
		double x;
		double y;
	};

	struct MouseScrollEvent {
		double xOffset;
		double yOffset;
	};

	struct CharEvent {
		unsigned int codepoint;
	};

	struct CharModsEvent {
		unsigned int codepoint;
		int mods;
	};

	struct FileDropEvent {
		int count;
		const char** paths;
	};
}
