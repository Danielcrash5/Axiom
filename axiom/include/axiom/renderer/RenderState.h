#pragma once
#include <glm/glm.hpp>

enum class BlendFactor {
    Zero,
    One,
    SrcAlpha,
    OneMinusSrcAlpha,
    SrcColor,
    OneMinusSrcColor
};

enum class CullMode {
    None,
    Back,
    Front,
    FrontAndBack
};

enum class DepthFunc {
    Less,
    LessEqual,
    Equal,
    Always,
    None
};

struct RenderState {
    // Depth
    bool DepthTest = false;
    DepthFunc DepthFunction = DepthFunc::Less;
    bool DepthWrite = true;

    // Blending
    bool Blending = false;
    BlendFactor BlendSrc = BlendFactor::SrcAlpha;
    BlendFactor BlendDst = BlendFactor::OneMinusSrcAlpha;

    // Culling
    bool CullFace = false;
    CullMode Cull = CullMode::Back;

    // Optional: Stencil / PolygonMode
    // bool StencilTest = false;
    // GLenum PolygonMode = GL_FILL;
};