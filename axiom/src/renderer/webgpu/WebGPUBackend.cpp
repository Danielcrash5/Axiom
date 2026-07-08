#include "axiom/renderer/webgpu/WebGPUBackend.h"
#include <iostream>

namespace axiom::renderer::rhi::webgpu {

    RHIResult<std::unique_ptr<WebGPUBackend>> WebGPUBackend::create() {
        auto backend = std::unique_ptr<WebGPUBackend>(new WebGPUBackend());

        wgpu::InstanceDescriptor instanceDesc {};
        backend->m_instance = wgpu::CreateInstance(&instanceDesc);
        if (!backend->m_instance) {
            return std::unexpected(RHIError::Unknown);
        }

        wgpu::RequestAdapterOptions adapterOpts {};
        adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;

        wgpu::Adapter adapter;
        wgpu::RequestAdapterCallbackInfo adapterCallbackInfo {};
        adapterCallbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
        adapterCallbackInfo.callback = [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter,
                                          const char* message, void* userdata) {
                                              auto* out = static_cast<wgpu::Adapter*>(userdata);
                                              if (status == WGPURequestAdapterStatus_Success) {
                                                  *out = wgpu::Adapter::Acquire(cAdapter);
                                              }
                                              else {
                                                  std::cerr << "Adapter request failed: " << (message ? message : "unknown") << "\n";
                                              }
            };
        adapterCallbackInfo.userdata = &adapter;

        auto future = backend->m_instance.RequestAdapter(&adapterOpts, adapterCallbackInfo);
        backend->m_instance.WaitAny(future, UINT64_MAX);

        if (!adapter) {
            return std::unexpected(RHIError::AdapterRequestFailed);
        }
        backend->m_adapter = std::move(adapter);

        wgpu::DeviceDescriptor deviceDesc {};
        deviceDesc.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
                std::cerr << "WebGPU Error (" << static_cast<int>(type) << "): "
                    << std::string_view(message) << "\n";
            });

        wgpu::Device device;
        wgpu::RequestDeviceCallbackInfo deviceCallbackInfo {};
        deviceCallbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
        deviceCallbackInfo.callback = [](WGPURequestDeviceStatus status, WGPUDevice cDevice,
                                         const char* message, void* userdata) {
                                             auto* out = static_cast<wgpu::Device*>(userdata);
                                             if (status == WGPURequestDeviceStatus_Success) {
                                                 *out = wgpu::Device::Acquire(cDevice);
                                             }
                                             else {
                                                 std::cerr << "Device request failed: " << (message ? message : "unknown") << "\n";
                                             }
            };
        deviceCallbackInfo.userdata = &device;

        auto deviceFuture = backend->m_adapter.RequestDevice(&deviceDesc, deviceCallbackInfo);
        backend->m_instance.WaitAny(deviceFuture, UINT64_MAX);

        if (!device) {
            return std::unexpected(RHIError::DeviceRequestFailed);
        }
        backend->m_device = std::move(device);
        backend->m_queue = backend->m_device.GetQueue();

        return backend;
    }

    RHIResult<BufferHandle> WebGPUBackend::createBuffer(const BufferDesc& desc) {
        if (desc.sizeBytes == 0) {
            return std::unexpected(RHIError::InvalidDescriptor);
        }

        wgpu::BufferUsage wgpuUsage = wgpu::BufferUsage::None;
        if (desc.usage & BufferUsage::Vertex)  wgpuUsage |= wgpu::BufferUsage::Vertex;
        if (desc.usage & BufferUsage::Index)   wgpuUsage |= wgpu::BufferUsage::Index;
        if (desc.usage & BufferUsage::Uniform) wgpuUsage |= wgpu::BufferUsage::Uniform;
        if (desc.usage & BufferUsage::Storage) wgpuUsage |= wgpu::BufferUsage::Storage;
        if (desc.usage & BufferUsage::CopySrc) wgpuUsage |= wgpu::BufferUsage::CopySrc;
        if (desc.usage & BufferUsage::CopyDst) wgpuUsage |= wgpu::BufferUsage::CopyDst;

        wgpu::BufferDescriptor wgpuDesc {};
        wgpuDesc.size = desc.sizeBytes;
        wgpuDesc.usage = wgpuUsage;
        wgpuDesc.label = desc.debugName.empty() ? nullptr : desc.debugName.data();

        wgpu::Buffer buffer = m_device.CreateBuffer(&wgpuDesc);
        if (!buffer) {
            return std::unexpected(RHIError::Unknown);
        }

        uint32_t index;
        if (!m_freeBufferSlots.empty()) {
            index = m_freeBufferSlots.back();
            m_freeBufferSlots.pop_back();
            m_buffers[index].buffer = std::move(buffer);
            m_buffers[index].alive = true;
            m_buffers[index].generation += 1;
        }
        else {
            index = static_cast<uint32_t>(m_buffers.size());
            m_buffers.push_back(BufferSlot { std::move(buffer), 1, true });
        }

        return BufferHandle { index, m_buffers[index].generation };
    }

    void WebGPUBackend::destroyBuffer(BufferHandle handle) {
        if (handle.index >= m_buffers.size()) return;
        auto& slot = m_buffers[handle.index];
        if (!slot.alive || slot.generation != handle.generation) return;

        slot.buffer = nullptr;
        slot.alive = false;
        m_freeBufferSlots.push_back(handle.index);
    }

    RHIResult<TextureHandle> WebGPUBackend::createTexture(const TextureDesc&) {
        return std::unexpected(RHIError::Unknown); // TODO
    }

    void WebGPUBackend::destroyTexture(TextureHandle) {
        // TODO
    }

    std::unique_ptr<CommandList> WebGPUBackend::createCommandList() {
        return nullptr; // TODO: sobald bindPipeline/draw gebraucht werden (Phase 3)
    }

    void WebGPUBackend::submit(CommandList&) {
        // TODO
    }

} // namespace axiom::renderer::rhi::webgpu