#pragma once
#include "axiom/renderer/RendererAPI.h"
#include "axiom/renderer/RenderstateCache.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/IndirectDrawBuffer.h"
#include <glad/glad.h>
#include <memory>

namespace axiom {
	class OpenGLRendererAPI : public RendererAPI {
	public:
		void Init() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void SetRenderState(const RenderState& state) override;
		void SetClearState(bool Depth, bool Color) override;

		void DrawIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) override;
		void DrawArrays(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) override;

		void DrawIndexedInstanced(const std::shared_ptr<VertexArray>& vao, uint32_t indexCount, uint32_t instanceCount, uint32_t offset = 0) override;
		void DrawArraysInstanced(const std::shared_ptr<VertexArray>& vao, uint32_t vertexCount, uint32_t instanceCount, uint32_t offset = 0) override;

		void DrawIndexedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) override;
		void DrawIndexedInstancedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) override;
		void DrawArraysInstancedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) override;

	private:
		GLbitfield ClearMask = 0;
		RenderStateCache m_Cache;
	};
}
