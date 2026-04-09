#include "axiom/platform/OpenGL/OpenGLRendererAPI.h"
#include <glad/glad.h>

namespace axiom {
	void OpenGLRendererAPI::Init() {
		glEnable(GL_BLEND);									// Alphablending aktivieren
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// Default Blendingfunktion setzen
		SetRenderState({});									// Default Renderstate setzen
		SetClearState(true, true);							// Default Clerstate setzen
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::SetClearState(bool Depth, bool Color) {
		if (Depth)
			ClearMask |= GL_DEPTH_BUFFER_BIT;

		if (Color)
			ClearMask |= GL_COLOR_BUFFER_BIT;
	}

	void OpenGLRendererAPI::Clear() {
		glClear(ClearMask);
	}

	GLenum toGL(DepthFunc depthFunc) {
		switch (depthFunc) {
		case DepthFunc::Less:
			return GL_LESS;
			break;
		case DepthFunc::LessEqual:
			return GL_LEQUAL;
			break;
		case DepthFunc::Equal:
			return GL_EQUAL;
			break;
		case DepthFunc::Always:
			return GL_ALWAYS;
			break;
		case DepthFunc::None:
			return GL_NONE;
			break;
		default:
			break;
		}
	}

	GLenum toGL(BlendFactor blendfactor) {
		switch (blendfactor) {
		case BlendFactor::Zero:
			return GL_ZERO;
			break;
		case BlendFactor::One:
			return GL_ONE;
			break;
		case BlendFactor::SrcAlpha:
			return GL_SRC_ALPHA;
			break;
		case BlendFactor::OneMinusSrcAlpha:
			return GL_ONE_MINUS_SRC_ALPHA;
			break;
		case BlendFactor::SrcColor:
			return GL_SRC_COLOR;
			break;
		case BlendFactor::OneMinusSrcColor:
			return GL_ONE_MINUS_SRC_COLOR;
			break;
		default:
			break;
		}
	}

	void OpenGLRendererAPI::SetRenderState(const RenderState& state) {
		if (!m_Cache.SetIfChanged(state))
			return;

		// Depth
		if (state.DepthTest) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(toGL(state.DepthFunction));
			glDepthMask(state.DepthWrite ? GL_TRUE : GL_FALSE);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}

		// Blending
		if (state.Blending) {
			glEnable(GL_BLEND);
			glBlendFunc(toGL(state.BlendSrc), toGL(state.BlendDst));
		}
		else {
			glDisable(GL_BLEND);
		}

		// Cull
		if (state.CullFace) {
			glEnable(GL_CULL_FACE);
			switch (state.Cull) {
			case CullMode::Back: glCullFace(GL_BACK); break;
			case CullMode::Front: glCullFace(GL_FRONT); break;
			case CullMode::FrontAndBack: glCullFace(GL_FRONT_AND_BACK); break;
			case CullMode::None: glDisable(GL_CULL_FACE); break;
			}
		}
		else {
			glDisable(GL_CULL_FACE);
		}
	}

	void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset) {
		vao->Bind();
		const void* offsetPtr = reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * sizeof(uint32_t)));
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, offsetPtr);
	}

	void OpenGLRendererAPI::DrawArrays(const std::shared_ptr<VertexArray>& vao, uint32_t count, uint32_t offset) {
		vao->Bind();
		glDrawArrays(GL_TRIANGLES, static_cast<GLint>(offset), static_cast<GLsizei>(count));
	}

	void OpenGLRendererAPI::DrawLinesIndexed(const std::shared_ptr<VertexArray>& vao,
											 uint32_t count,
											 uint32_t offset) {
		vao->Bind();
		const void* offsetPtr = reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * sizeof(uint32_t)));
		glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, offsetPtr);
	}
}
