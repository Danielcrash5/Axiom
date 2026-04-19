#pragma once
#include "axiom/renderer/IndirectDrawBuffer.h"
#include <vector>
#include <cstdint>

namespace axiom {
	class OpenGLIndirectDrawBuffer : public IndirectDrawBuffer {
	public:
		OpenGLIndirectDrawBuffer();
		~OpenGLIndirectDrawBuffer() override;

		void SetCommands(const std::vector<DrawElementsIndirectCommand>& commands) override;
		void Bind() const override;
		void Unbind() const override;
		const DrawElementsIndirectCommand& GetCommand(uint32_t index) const override;
		uint32_t GetCount() const override;

	private:
		uint32_t m_BufferID = 0;
		std::vector<DrawElementsIndirectCommand> m_Commands;
	};
}
