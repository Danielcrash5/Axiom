#include "axiom/Axiom.h"
#include <cstdlib>

namespace axiom {

Application* CreateApplication();

}

int main(int argc, char** argv) {
    axiom::Application* app = axiom::CreateApplication();
    if (!app)
        return EXIT_FAILURE;

    app->Run(argc, argv);
    delete app;
    return EXIT_SUCCESS;
}
