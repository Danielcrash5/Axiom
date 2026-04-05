#pragma once
#include "RendererAPI.h"
#include <memory>

namespace axiom {
	class VertexArray;

	class RenderCommand {
	public:
		static void Init();

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

		static void DrawLinesIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) {
			s_RendererAPI->DrawLinesIndexed(vao, count, offset);
		}

		static void SetRenderState(RenderState& renderstate) {
			s_RendererAPI->SetRenderState(renderstate);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}