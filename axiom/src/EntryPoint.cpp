#include "axiom/Axiom.h"
#include <cstdlib>

namespace axiom {
Application *CreateApplication();
}

// WEG A: Konsole ist aktiviert ODER wir sind NICHT auf Windows -> Standard main
#if defined(AXIOM_ENABLE_CONSOLE_LOG) || !defined(_WIN32)

int main(int argc, char **argv) {
    axiom::Application *app = axiom::CreateApplication();
    if (!app)
        return EXIT_FAILURE;

    app->Run(argc, argv);
    delete app;
    return EXIT_SUCCESS;
}

// WEG B: Windows ist aktiv UND Konsole ist deaktiviert -> WinMain
#else

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    // Da WinMain keine argc/argv liefert, nutzen wir die Standard-C-Bibliothek,
    // um an die Argumente f�r deine app->Run() Methode zu kommen:
    int argc = __argc;
    char **argv = __argv;

    axiom::Application *app = axiom::CreateApplication();
    if (!app)
        return EXIT_FAILURE;

    app->Run(argc, argv);
    delete app;
    return EXIT_SUCCESS;
}

#endif
