#pragma once

#include <string_view>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

struct SDL_Window;

namespace axiom {

    enum class VertexFormat {
        Vec2,
        Vec3,
        Vec4,
        ColorRGBA8 // Packed uint32_t -> konvertiert zu vec4 UNORM im Shader
    };

    struct VertexElement {
        uint32_t location;
        VertexFormat format;
        uint32_t offset;
    };

    struct VertexLayout {
        uint32_t stride = 0;
        std::vector<VertexElement> elements;
    };

    enum class ShaderStage : uint32_t {
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Compute = 1 << 2,
        Geometry = 1 << 3,
        Mesh = 1 << 4,
        Amplification = 1 << 5
    };

    enum class DescriptorType {
        UniformBuffer,
        StorageBuffer,    // SSBO (wichtig für Compute / GPU-driven Rendering)
        ImageSampler      // Texturen
    };

    // Beschreibt ein einzelnes reflektiertes Binding (z.B. ein Set/Binding-Slot)
    struct ResourceBindingDesc {
        uint32_t set;
        uint32_t binding;
        DescriptorType type;
        uint32_t stageFlags; // Bitmaske aus ShaderStage
        uint32_t size;       // Puffer-Größe (0 für Texturen)
        std::string name;
    };

    enum class PipelineType {
        Graphics,
        Compute
    };

    enum class PrimitiveTopology {
        Triangles,
        Lines,
        Points
    };

    enum class BlendMode {
        None,
        AlphaBlend,
        Additive
    };

    struct ShaderModuleDesc {
        ShaderStage stage;
        std::vector<uint32_t> spirvCode;
        std::string entryPoint = "main";
    };

    // Das universelle, datengetriebene Pipeline-Struct
    struct PipelineStateDesc {
        PipelineType type = PipelineType::Graphics;
        std::vector<ShaderModuleDesc> stages;

        // Reflection füllt diese Bindings automatisch aus!
        std::vector<ResourceBindingDesc> reflectedBindings;

        // Nur für Graphics relevant
        VertexLayout vertexLayout;
        PrimitiveTopology topology = PrimitiveTopology::Triangles;
        BlendMode blendMode = BlendMode::AlphaBlend;

        uint32_t pushConstantSize = 0;
    };

    using PipelineHandle = uint32_t;

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
        void begin_rendering(float r, float g, float b, float a);
        void end_rendering();

        void bind_pipeline(PipelineHandle pipeline);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        // Backend-Schnittstelle zum Setzen des nativen Handles
        void set_native_handle(void* cmdBuffer, GraphicsDevice* device, uint32_t swapchainImageIndex);

    private:
        CommandBufferImpl* m_impl = nullptr;
    };

    using ShaderModuleHandle = void*;

    class GraphicsDevice {
    public:
        GraphicsDevice(SDL_Window* window);
        ~GraphicsDevice();

        PipelineHandle create_pipeline(const PipelineStateDesc& desc);

        bool begin_frame(CommandBuffer& outCmdBuffer);
        void end_frame();
        void handle_resize(int newWidth, int newHeight);

        ShaderModuleHandle create_shader_module(const ShaderModuleDesc& desc);
        void destroy_shader_module(ShaderModuleHandle handle);

        BufferHandle create_buffer(uint64_t size, BufferUsage usage);
        TextureHandle create_texture(TextureFormat format, uint32_t width, uint32_t height);
        void submit(CommandBuffer& cmd);

        TransientAllocation allocate_transient(uint64_t size, uint32_t alignment = 16);

        // Liefert das rohe VkBuffer-Handle direkt als void* zurück
        void* get_native_buffer_handle(BufferHandle handle);

        // Gibt die aktuelle Ausdehnung der Swapchain zurück
        void get_swapchain_extent(uint32_t& outWidth, uint32_t& outHeight);

        void* get_native_pipeline(PipelineHandle handle);
        void* get_native_pipeline_layout(PipelineHandle handle);
        void* get_current_swapchain_image();
        void* get_current_swapchain_image_view();

    private:
        GraphicsDeviceImpl* m_impl = nullptr;
    };

} // namespace axiom