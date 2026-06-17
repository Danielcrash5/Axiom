#pragma once

#include "../resources/handles.h"

namespace axiom {
    class RenderContext {
    public:

        virtual ~RenderContext() = default;

        virtual void bind_pipeline(
            PipelineHandle pipeline) = 0;

        virtual void bind_vertex_buffer(
            BufferHandle buffer) = 0;

        virtual void bind_index_buffer(
            BufferHandle buffer) = 0;

        virtual void draw(
            uint32_t vertex_count) = 0;

        virtual void draw_indexed(
            uint32_t index_count) = 0;
    };
}