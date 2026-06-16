#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#include <glm/glm.hpp>

namespace axiom {
	// Starke Typisierung für Ressourcen-Handles via IDs (für dein Hybrid-Bindless-System)
	using TextureHandle = uint32_t;
	using BufferHandle = uint32_t;
	inline constexpr TextureHandle INVALID_TEXTURE = 0xFFFFFFFF;
	inline constexpr BufferHandle  INVALID_BUFFER = 0xFFFFFFFF;

	enum class BufferUsage : uint32_t {
		Vertex = 1 << 0,
		Index = 1 << 1,
		Uniform = 1 << 2,
		Storage = 1 << 3,
		Indirect = 1 << 4
	};

	enum class TextureFormat {
		RGBA8,
		BGRA8,
		Depth32,
		Depth24Stencil8
	};

	enum class VertexFormat {
		Float, Vec2, Vec3, Vec4,
		Int, IVec2, UVec4, UInt, UByte4N
	};

	constexpr uint32_t get_vertex_format_size(VertexFormat format) {
		switch (format) {
		case VertexFormat::Float:   return sizeof(float);
		case VertexFormat::Vec2:    return sizeof(glm::vec2);
		case VertexFormat::Vec3:    return sizeof(glm::vec3);
		case VertexFormat::Vec4:    return sizeof(glm::vec4);
		case VertexFormat::Int:     return sizeof(int32_t);
		case VertexFormat::IVec2:   return sizeof(glm::ivec2);
		case VertexFormat::UVec4:   return sizeof(glm::uvec4);
		case VertexFormat::UInt:    return sizeof(uint32_t);
		case VertexFormat::UByte4N: return 4;
		}
		return 0;
	}

	struct VertexAttribute {
		uint32_t location;
		VertexFormat format;
		uint32_t offset;
	};

	struct VertexLayout {
		std::vector<VertexAttribute> attributes;
		uint32_t stride = 0;

		void add_attribute(uint32_t location, VertexFormat format) {
			attributes.push_back({ .location = location, .format = format, .offset = stride });
			stride += get_vertex_format_size(format);
		}
	};
}
