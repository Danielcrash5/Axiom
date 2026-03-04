#include <axiom/core/Application.h>
#include <axiom/core/Layer.h>
#include <axiom/input/KeyCodes.h>
#include <axiom/core/Logger.h>
#include <iostream>

class InputTestLayer : public axiom::Layer {
public:
	InputTestLayer() :
		Layer("InputTestLayer") {}
	void OnAttach() override {
		AXIOM_INFO("InputTestLayer attached!");
	}
	void OnFixedUpdate(float dt) override {
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

	}
protected:
	void OnInit() override {
		PushLayer(std::make_unique<InputTestLayer>());
	}

	void OnUpdate(float dt) override {}

	void OnShutdown() override {

	}
};

int main() {
	Testbed game;
	game.Run();
	return 0;
}