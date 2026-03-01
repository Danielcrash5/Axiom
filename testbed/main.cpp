#include <axiom/core/Application.h>
#include <axiom/core/Layer.h>
#include <axiom/input/KeyCodes.h>
#include <iostream>

class InputTestLayer : public axiom::Layer {
    public:
    InputTestLayer() :
        Layer("InputTestLayer") {
    }
    void OnAttach() override {
        std::cout << "InputTestLayer attached!" << std::endl;
    }
    void OnUpdate(float dt) override {
        auto input = GetMainInput();
        if (input.IsKeyPressed(axiom::Key::Space)) {
            std::cout << "Space key is pressed!" << std::endl;
        }
    }
    void OnDetach() override {
        std::cout << "InputTestLayer detached!" << std::endl;
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

    void OnUpdate(float dt) override {
    }

    void OnShutdown() override {
       
    }
};

int main() {
    Testbed game;
    game.Run();
    return 0;
}