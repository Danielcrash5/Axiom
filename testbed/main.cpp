#include <axiom/core/Application.h>
#include <iostream>

class Testbed : public axiom::Application {
public:
    Testbed() :
        Application("testbed") {

    }
protected:
    void OnInit() override {
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