#pragma once
#include <memory>
#include <string_view>

struct SDL_Window;

namespace axiom {

    class GraphicsDevice; // Vorwärtsdeklaration
    class CommandBuffer;

    class Renderer {
    public:
        Renderer(SDL_Window* window);
        ~Renderer();

        // Verhindert Kopieren, da der Renderer die Hardware-Ressourcen besitzt
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // Der zentrale Engine-Loop Aufruf
        bool begin_frame(CommandBuffer& outCmdBuffer);
        void end_frame();

        void on_window_resize(int newWidth, int newHeight);

        // Zugriff auf das Hardware-Backend für andere Engine-Systeme (z.B. RmlUI)
        GraphicsDevice& get_device();

    private:
        SDL_Window* m_window = nullptr;
        std::unique_ptr<GraphicsDevice> m_device;
    };

} // namespace Axiom