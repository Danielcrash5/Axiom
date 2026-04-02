#pragma once
#include <glm/glm.hpp>
#include "RenderState.h"

enum class RendererAPIType {
    None = 0,
    OpenGL = 1
};

class RendererAPI {
public:
    virtual ~RendererAPI() = default;

    virtual void Init() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;
    virtual void SetRenderState(const RenderState& state) = 0;

    virtual void DrawIndexed(uint32_t count) = 0;

    static RendererAPIType GetAPI() {
        return s_API;
    }

protected:
    static RendererAPIType s_API;
};