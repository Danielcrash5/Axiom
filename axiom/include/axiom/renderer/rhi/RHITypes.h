#pragma once
#include <cstdint>
#include <string_view>
#include <expected>
#include "Handle.h"

namespace axiom::renderer::rhi {

    enum class RHIError : uint8_t {
        DeviceLost,
        OutOfMemory,
        InvalidDescriptor,
        AdapterRequestFailed,
        DeviceRequestFailed,
        Unknown,
    };

    template <typename T>
    using RHIResult = std::expected<T, RHIError>;

    enum class TextureFormat : uint8_t {
        RGBA8Unorm,
        BGRA8Unorm,
        Depth32Float,
        R8Unorm,
    };

    enum class BufferUsage : uint32_t {
        None = 0,
        Vertex = 1u << 0,
        Index = 1u << 1,
        Uniform = 1u << 2,
        Storage = 1u << 3,
        CopySrc = 1u << 4,
        CopyDst = 1u << 5,
    };
    [[nodiscard]] constexpr BufferUsage operator|(BufferUsage a, BufferUsage b) noexcept {
        return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    [[nodiscard]] constexpr bool operator&(BufferUsage a, BufferUsage b) noexcept {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    enum class TextureUsage : uint32_t {
        None = 0,
        RenderTarget = 1u << 0,
        Sampled = 1u << 1,
        CopySrc = 1u << 2,
        CopyDst = 1u << 3,
    };
    [[nodiscard]] constexpr TextureUsage operator|(TextureUsage a, TextureUsage b) noexcept {
        return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    [[nodiscard]] constexpr bool operator&(TextureUsage a, TextureUsage b) noexcept {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    struct BufferDesc {
        uint64_t sizeBytes = 0;
        BufferUsage usage = BufferUsage::None;
        std::string_view debugName;
    };

    struct TextureDesc {
        uint32_t width = 0;
        uint32_t height = 0;
        TextureFormat format = TextureFormat::RGBA8Unorm;
        TextureUsage usage = TextureUsage::None;
        std::string_view debugName;
    };

} // namespace axiom::renderer::rhi