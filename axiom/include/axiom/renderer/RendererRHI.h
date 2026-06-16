#pragma once

#include <string_view>
#include <cstdint>
#include <memory>

struct SDL_Window;

namespace axiom {

    struct CommandBufferImpl;
    struct GraphicsDeviceImpl;

    enum class BufferUsage : uint32_t {
        Vertex = 1 << 0,
        Index = 1 << 1,
        Uniform = 1 << 2,
        Storage = 1 << 3,
        Indirect = 1 << 4
    };

    using BufferHandle = uint32_t;
    using TextureHandle = uint32_t;
    enum class TextureFormat {
        RGBA8
    };

    struct TransientAllocation {
        BufferHandle bufferHandle;
        uint64_t offset;
        void* pMappedData;
    };

    class GraphicsDevice;

    class CommandBuffer {
    public:
        CommandBuffer();
        ~CommandBuffer();

        void bind_vertex_buffer(BufferHandle buf, uint64_t offset);
        void bind_index_buffer(BufferHandle buf, uint64_t offset);
        void draw_indexed_indirect(BufferHandle indirectBuffer, uint64_t offset, uint32_t drawCount);
        void push_constants(const void* data, uint32_t size);

        void begin_rendering(TextureHandle swapchainTexture, float r, float g, float b, float a);
        void end_rendering();

        // Backend-Schnittstelle zum Setzen des nativen Handles
        void set_native_handle(void* cmdBuffer, GraphicsDevice* device, uint32_t swapchainImageIndex);

    private:
        CommandBufferImpl* m_impl = nullptr;
    };

    class GraphicsDevice {
    public:
        GraphicsDevice(SDL_Window* window);
        ~GraphicsDevice();

        bool begin_frame(CommandBuffer& outCmdBuffer);
        void end_frame();
        void handle_resize(int newWidth, int newHeight);

        BufferHandle create_buffer(uint64_t size, BufferUsage usage);
        TextureHandle create_texture(TextureFormat format, uint32_t width, uint32_t height);
        void submit(CommandBuffer& cmd);

        TransientAllocation allocate_transient(uint64_t size, uint32_t alignment = 16);

        // Liefert das rohe VkBuffer-Handle direkt als void* zurück
        void* get_native_buffer_handle(BufferHandle handle);

        // Gibt die aktuelle Ausdehnung der Swapchain zurück
        void get_swapchain_extent(uint32_t& outWidth, uint32_t& outHeight);

        void* get_current_swapchain_image_view();
        void* get_current_swapchain_image();

    private:
        GraphicsDeviceImpl* m_impl = nullptr;
    };

} // namespace axiom