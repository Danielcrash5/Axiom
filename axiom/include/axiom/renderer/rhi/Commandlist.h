#pragma once
#include "Handle.h"

namespace axiom::renderer::rhi {

    class CommandList {
    public:
        virtual ~CommandList() = default;
        virtual void copyBufferToBuffer(BufferHandle src, uint64_t srcOffset,
                                        BufferHandle dst, uint64_t dstOffset,
                                        uint64_t sizeBytes) = 0;
    };

} // namespace axiom::renderer::rhi