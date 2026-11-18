/**
 * @file axiom/include/axiom/renderer/rhi/VertexLayout.h
 * @brief Declares the public interface for the vertex layout.
 */

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace axiom::renderer::rhi {

/**
 * @brief Defines the available vertex format values.
 * @tparam VertexFormat Template type used by this type.
 */
enum class VertexFormat { float32, float32x2, float32x3, float32x4, uint32, uint8x4Norm };

/** @brief Stores the data for the vertex attribute. */
struct VertexAttribute {
    /** @brief Stores the semantic. */
    std::string semantic; // z.B. "POSITION", "TEXCOORD0", "CUSTOM_W" fuer 4D+
    /** @brief Stores the location. */
    uint32_t location = 0;
    /** @brief Stores the format. */
    VertexFormat format = VertexFormat::float32x3;
    /** @brief Stores the offset. */
    uint32_t offset = 0;
};

/** @brief Stores the data for the vertex layout. */
struct VertexLayout {
    /** @brief Stores the stride. */
    uint32_t stride = 0;
    /** @brief Stores the attributes. */
    std::vector<VertexAttribute> attributes;
};

} // namespace axiom::renderer::rhi
