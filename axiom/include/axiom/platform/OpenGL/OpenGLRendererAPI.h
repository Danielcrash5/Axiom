#pragma once
#include "axiom/renderer/RendererAPI.h"
#include "axiom/renderer/RenderstateCache.h"
#include "axiom/renderer/VertexArray.h"
#include <glad/glad.h>

namespace axiom {
	class OpenGLRendererAPI : public RendererAPI {
	public:
		void Init() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void SetRenderState(const RenderState& state);
		void SetClearState(bool Depth, bool Color);

		void DrawIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) override;
		void DrawArrays(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) override;
		void DrawLinesIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset) override;


	private:
		GLbitfield ClearMask;
		RenderStateCache m_Cache;
	};
}
