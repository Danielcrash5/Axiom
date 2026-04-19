#include "axiom/platform/OpenGL/OpenGLRendererAPI.h"
#include "axiom/renderer/IndirectDrawBuffer.h"
#include "axiom/core/Logger.h"
#include <glad/glad.h>
#include <iostream>

namespace axiom {

void GLAPIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*) {
    const char* _type = "OTHER";
    if (type == GL_DEBUG_TYPE_ERROR) _type = "ERROR";
    else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) _type = "DEPRECATED";
    else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) _type = "UNDEFINED";
    else if (type == GL_DEBUG_TYPE_PORTABILITY) _type = "PORTABILITY";
    else if (type == GL_DEBUG_TYPE_PERFORMANCE) _type = "PERFORMANCE";
    else if (type == GL_DEBUG_TYPE_MARKER) _type = "MARKER";
    else if (type == GL_DEBUG_TYPE_PUSH_GROUP) _type = "PUSH_GROUP";
    else if (type == GL_DEBUG_TYPE_POP_GROUP) _type = "POP_GROUP";

    if (severity == GL_DEBUG_SEVERITY_HIGH)
        AXIOM_ERROR("[OpenGL][{}][{}] {}", _type, id, message);
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
        AXIOM_WARN("[OpenGL][{}][{}] {}", _type, id, message);
    else
        AXIOM_INFO("[OpenGL][{}][{}] {}", _type, id, message);
}

	void OpenGLRendererAPI::Init() {
		SetClearState(true, true);							// Default Clearstate setzen
		SetRenderState({});									// Default Renderstate setzen

		// Enable OpenGL debug output if available
		if (GLAD_GL_KHR_debug) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLDebugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::SetClearState(bool Depth, bool Color) {
		ClearMask = 0;

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
		case DepthFunc::LessEqual:
			return GL_LEQUAL;
		case DepthFunc::Equal:
			return GL_EQUAL;
		case DepthFunc::Always:
			return GL_ALWAYS;
		case DepthFunc::None:
			return GL_ALWAYS;
		}

		return GL_LESS;
	}

	GLenum toGL(BlendFactor blendfactor) {
		switch (blendfactor) {
		case BlendFactor::Zero:
			return GL_ZERO;
		case BlendFactor::One:
			return GL_ONE;
		case BlendFactor::SrcAlpha:
			return GL_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha:
			return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::SrcColor:
			return GL_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor:
			return GL_ONE_MINUS_SRC_COLOR;
		}

		return GL_ONE;
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

	void OpenGLRendererAPI::DrawIndexedInstanced(const std::shared_ptr<VertexArray>& vao, uint32_t indexCount, uint32_t instanceCount, uint32_t offset) {
		vao->Bind();
		const void* offsetPtr = reinterpret_cast<const void*>(static_cast<uintptr_t>(offset * sizeof(uint32_t)));
		glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, offsetPtr, instanceCount);
	}

	void OpenGLRendererAPI::DrawArraysInstanced(const std::shared_ptr<VertexArray>& vao, uint32_t vertexCount, uint32_t instanceCount, uint32_t offset) {
		vao->Bind();
		glDrawArraysInstanced(GL_TRIANGLES, static_cast<GLint>(offset), static_cast<GLsizei>(vertexCount), static_cast<GLsizei>(instanceCount));
	}

	void OpenGLRendererAPI::DrawIndexedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) {
		vao->Bind();
		indirectBuffer->Bind();
		if (GLAD_GL_ARB_multi_draw_indirect && GLAD_GL_ARB_draw_indirect) {
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, drawCount, 0);
		} else {
			// Fallback: Einzelne Draws ausführen
			for (uint32_t i = 0; i < drawCount; ++i) {
				const DrawElementsIndirectCommand& cmd = indirectBuffer->GetCommand(i);
				glDrawElementsInstancedBaseVertexBaseInstance(
					GL_TRIANGLES, cmd.count, GL_UNSIGNED_INT, reinterpret_cast<void*>(cmd.firstIndex * sizeof(uint32_t)),
					cmd.instanceCount, cmd.baseVertex, cmd.baseInstance);
			}
		}
		indirectBuffer->Unbind();
	}

	void OpenGLRendererAPI::DrawIndexedInstancedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) {
		DrawIndexedIndirect(vao, indirectBuffer, drawCount);
	}

	void OpenGLRendererAPI::DrawArraysInstancedIndirect(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<IndirectDrawBuffer>& indirectBuffer, uint32_t drawCount) {
		// TODO: Implementiere DrawArraysInstancedIndirect analog zu DrawIndexedIndirect
	}
}
