#include <axiom/core/Application.h>
#include <iostream>

class MyGame : public axiom::Application {
protected:
    void OnInit() override {
        std::cout << "Init\n";
    }

    void OnUpdate(float dt) override {
        std::cout << "Frame: " << dt << "\n";
    }

    void OnShutdown() override {
        std::cout << "Shutdown\n";
    }
};

int main() {
    MyGame game;
    game.Run();
}