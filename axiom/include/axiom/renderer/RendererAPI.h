#pragma once
#include <glm/glm.hpp>
#include "RenderState.h"
#include <memory>
#include "VertexArray.h"

namespace axiom {
	enum class RendererAPIType {
		None = 0,
		OpenGL = 1
	};

	class IndirectDrawBuffer; // Forward declaration

	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void SetClearState(bool Depth, bool Color) = 0;
		virtual void Clear() = 0;
		virtual void SetRenderState(const RenderState& state) = 0;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset) = 0;
		virtual void DrawArrays(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset) = 0;

		// Instanced rendering
		virtual void DrawIndexedInstanced(const std::shared_ptr<VertexArray>& vao, uint32_t indexCount, uint32_t instanceCount, uint32_t offset = 0) = 0;
		virtual void DrawArraysInstanced(const std::shared_ptr<VertexArray>& vao, uint32_t vertexCount, uint32_t instanceCount, uint32_t offset = 0) = 0;

		// Indirect rendering (API-agnostisch)
		virtual void DrawIndexedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) = 0;
		virtual void DrawIndexedInstancedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) = 0;
		virtual void DrawArraysInstancedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) = 0;
		virtual bool SupportsIndirectRendering() const = 0;

		static RendererAPIType GetAPI() {
			return s_API;
		}

	protected:
		static RendererAPIType s_API;
	};
}
