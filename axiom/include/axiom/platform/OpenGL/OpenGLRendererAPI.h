#pragma once
#include "axiom/renderer/RendererAPI.h"
#include "axiom/renderer/RenderstateCache.h"
#include "axiom/renderer/VertexArray.h"

namespace axiom {
	class OpenGLRendererAPI : public RendererAPI {
	public:
		void Init() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void SetRenderState(const RenderState& state);

		void DrawIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset = 0) override;


	private:
		RenderStateCache m_Cache;
	};
}