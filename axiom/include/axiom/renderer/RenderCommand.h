#pragma once
#include "RendererAPI.h"
#include "IndirectDrawBuffer.h"
#include <memory>

namespace axiom {
	class VertexArray;

	class RenderCommand {
	public:
		static void Init() {
			s_RendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color) {
			s_RendererAPI->SetClearColor(color);
		}

		static void SetClearState(bool Depth, bool Color) {
			s_RendererAPI->SetClearState(Depth, Color);
		}

		static void Clear() {
			s_RendererAPI->Clear();
		}

		static void DrawIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) {
			s_RendererAPI->DrawIndexed(vao, count, offset);
		}

		static void DrawArrays(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) {
			s_RendererAPI->DrawArrays(vao, count, offset);
		}

		static void DrawIndexedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) {
			s_RendererAPI->DrawIndexedIndirect(vao, indirectBuffer, drawCount);
		}

		static bool SupportsIndirectRendering() {
			return s_RendererAPI->SupportsIndirectRendering();
		}

		static void SetRenderState(RenderState& renderstate) {
			s_RendererAPI->SetRenderState(renderstate);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}
