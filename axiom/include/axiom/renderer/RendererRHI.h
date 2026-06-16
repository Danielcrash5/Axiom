#pragma once
#include "RHICommon.h"
#include <expected>

struct SDL_Window;

namespace axiom {

    // Vorwärtsdeklaration für den konkreten Zustand (wird in der CPP definiert)
    struct GraphicsDeviceImpl;
    struct CommandBufferImpl;

    class CommandBuffer {
    public:
        CommandBuffer();
        ~CommandBuffer();

        void bind_vertex_buffer(BufferHandle buf, uint64_t offset);
        void bind_index_buffer(BufferHandle buf, uint64_t offset);
        void draw_indexed_indirect(BufferHandle indirectBuffer, uint64_t offset, uint32_t drawCount);
        void push_constants(const void* data, uint32_t size);

        // Erlaubt dem Backend, an die echten Daten zu kommen
        CommandBufferImpl* get_impl() {
            return m_impl;
        }

    private:
        CommandBufferImpl* m_impl = nullptr;
    };

    class GraphicsDevice {
    public:
        GraphicsDevice(SDL_Window* window);
        ~GraphicsDevice();

        BufferHandle create_buffer(uint64_t size, BufferUsage usage);
        TextureHandle create_texture(TextureFormat format, uint32_t width, uint32_t height);

        bool begin_frame();
        void end_frame();
        void submit(CommandBuffer& cmd);
        void handle_resize(int newWidth, int newHeight);

    private:
        GraphicsDeviceImpl* m_impl = nullptr;
    };

} // namespace Axiom