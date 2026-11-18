/**
 * @file axiom/include/axiom/renderer/rendergraph/RenderGraphTypes.h
 * @brief Declares the public interface for the render graph types.
 */

#pragma once
#include <axiom/renderer/rhi/RHITypes.h>
#include <cstdint>
#include <string_view>

namespace axiom::renderer::rendergraph {

    /**
     * @brief Defines the available resource type values.
     * @tparam ResourceType Template type used by this type.
     */
    enum class ResourceType { texture, buffer };

    /** @brief Stores the data for the texture resource desc. */
    struct TextureResourceDesc {
        /** @brief Stores the width. */
        uint32_t width = 0;
        /** @brief Stores the height. */
        uint32_t height = 0;
        /** @brief Stores the format. */
        rhi::TextureFormat format = rhi::TextureFormat::rgbA8Unorm;
        /** @brief Stores the usage. */
        rhi::TextureUsage usage = rhi::TextureUsage::none;
        /** @brief Stores the debug name. */
        std::string_view debugName;
    };

    // Fuer Dependency-Aufloesung: was macht der Pass mit der Resource.
    // Write waehlt das konkrete Layout anhand der TextureUsage:
    // RenderTarget -> ColorAttachment, sonst TransferDst.
    /**
     * @brief Defines the available access type values.
     * @tparam AccessType Template type used by this type.
     */
    enum class AccessType { read, write };

} // namespace axiom::renderer::rendergraph
