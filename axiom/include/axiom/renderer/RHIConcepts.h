#pragma once
#include "RHICommon.h"
#include <expected>

namespace axiom {
	enum class RHIError {
		InitializationFailed,
		DeviceLost,
		OutOfMemory,
		InvalidShader
	};

	// Vorwärtsdeklarationen für die Compile-Time-Checks
	struct PipelineDesc;

	template<typename T>
	concept ICommandBuffer = requires(T cmd, BufferHandle buf, uint64_t offset, uint32_t count, TextureHandle tex) {
		{
			cmd.bind_vertex_buffer(buf, offset)
		} -> std::same_as<void>;
		{
			cmd.bind_index_buffer(buf, offset)
		} -> std::same_as<void>;
		{
			cmd.draw_indexed_indirect(buf, offset, count)
		} -> std::same_as<void>;
		{
			cmd.push_constants(static_cast<const void*>(nullptr), uint32_t {})
		} -> std::same_as<void>;
	};

	template<typename T, typename CmdBufferType>
	concept IGraphicsDevice = requires(T device, CmdBufferType & cmd, const PipelineDesc & desc) {
		{
			device.create_buffer(uint64_t {}, BufferUsage::Vertex)
		} -> std::same_as<BufferHandle>;
		{
			device.create_texture(TextureFormat::RGBA8, uint32_t {}, uint32_t {})
		} -> std::same_as<TextureHandle>;
		{
			device.begin_frame()
		} -> std::same_as<bool>;
		{
			device.end_frame()
		} -> std::same_as<void>;
		{
			device.submit(cmd)
		} -> std::same_as<void>;
	};
}