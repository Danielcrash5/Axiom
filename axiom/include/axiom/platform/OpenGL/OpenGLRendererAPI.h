#pragma once
#include "axiom/renderer/RendererAPI.h"

class OpenGLRendererAPI : public RendererAPI {
public:
    void Init() override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

    void SetClearColor(const glm::vec4& color) override;
    void Clear() override;

    void DrawIndexed(uint32_t count) override;
};