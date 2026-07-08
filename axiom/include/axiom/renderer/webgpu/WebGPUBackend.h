#pragma once
#include <webgpu/webgpu_cpp.h>
#include <vector>
#include <axiom/renderer/rhi/IRHIBackend.h>

namespace axiom::renderer::rhi::webgpu {

    class WebGPUBackend final : public IRHIBackend {
    public:
        [[nodiscard]] static RHIResult<std::unique_ptr<WebGPUBackend>> create();

        RHIResult<BufferHandle> createBuffer(const BufferDesc&) override;
        RHIResult<TextureHandle> createTexture(const TextureDesc&) override;
        void destroyBuffer(BufferHandle) override;
        void destroyTexture(TextureHandle) override;
        std::unique_ptr<CommandList> createCommandList() override;
        void submit(CommandList&) override;

    private:
        WebGPUBackend() = default;

        wgpu::Instance m_instance;
        wgpu::Adapter  m_adapter;
        wgpu::Device   m_device;
        wgpu::Queue    m_queue;

        struct BufferSlot {
            wgpu::Buffer buffer; uint32_t generation = 0; bool alive = false;
        };
        std::vector<BufferSlot> m_buffers;
        std::vector<uint32_t> m_freeBufferSlots;
    };

} // namespace axiom::renderer::rhi::webgpu