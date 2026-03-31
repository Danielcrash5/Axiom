#include <axiom/core/Application.h>
#include <axiom/core/Layer.h>
#include <axiom/input/KeyCodes.h>
#include <axiom/core/Logger.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "scene/TestScene.h"

class InputTestLayer : public axiom::Layer {
public:
	InputTestLayer() :
		Layer("InputTestLayer") {}
	void OnAttach() override {
		AXIOM_INFO("InputTestLayer attached!");
	}
	void OnFixedUpdate(double dt) override {
		auto input = GetMainInput();
		if (input.IsKeyPressed(axiom::Key::Space)) {
			AXIOM_INFO("Space key is pressed!");
		}
	}
	void OnDetach() override {
		AXIOM_INFO("InputTestLayer detached!");
	}
};

class Testbed : public axiom::Application {
public:
	Testbed() :
		Application("testbed") {
		// Nie etwas hier hinein schreiben
	}
protected:
	void OnInit() override {
        PushLayer(std::make_unique<InputTestLayer>());
		PushLayer(std::make_unique<TestScene>());
	}

	void OnRender(double alpha) override {
		
	}

	void OnUpdate(double dt) override {
		
	}

	void OnShutdown() override {
		
	}

	
};

int main() {
	Testbed game;
	game.Run();
	return 0;
}