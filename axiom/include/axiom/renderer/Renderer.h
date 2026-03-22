#pragma once

namespace axiom {

    class Renderer {
    public:

        static void Init();
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();

        static void Clear();

    };

}