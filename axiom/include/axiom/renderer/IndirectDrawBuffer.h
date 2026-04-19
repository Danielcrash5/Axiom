#pragma once
#include <vector>
#include <memory>
#include <cstdint>

namespace axiom {
	struct DrawElementsIndirectCommand {
		uint32_t count;
		uint32_t instanceCount;
		uint32_t firstIndex;
		uint32_t baseVertex;
		uint32_t baseInstance;
	};

	class IndirectDrawBuffer {
	public:
		virtual ~IndirectDrawBuffer() = default;
		virtual void SetCommands(const std::vector<DrawElementsIndirectCommand>& commands) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual const DrawElementsIndirectCommand& GetCommand(uint32_t index) const = 0;
		virtual uint32_t GetCount() const = 0;

		static std::shared_ptr<IndirectDrawBuffer> Create();
	};
}
