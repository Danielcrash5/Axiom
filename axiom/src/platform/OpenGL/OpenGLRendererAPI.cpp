#include "axiom/platform/OpenGL/OpenGLRendererAPI.h"
#include <glad/glad.h>

void OpenGLRendererAPI::Init() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SetRenderState({}); //Default Renderstate setzen
}

void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	glViewport(x, y, width, height);
}

void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::Clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::SetRenderState(const RenderState& state) {
	if (!m_Cache.SetIfChanged(state))
		return;

	// Depth
	state.DepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);

	// Blend
	if (state.Blending) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
		glDisable(GL_BLEND);

	// Cull
	state.CullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void OpenGLRendererAPI::DrawIndexed(uint32_t count) {
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}