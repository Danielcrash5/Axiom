#pragma once
#include <glad/glad.h>

namespace axiom {

    struct RenderState {
        // Depth
        bool DepthTest = true;
        bool DepthWrite = true;
        GLenum DepthFunc = GL_LESS;

        // Blending
        bool Blend = true;
        GLenum BlendSrc = GL_SRC_ALPHA;
        GLenum BlendDst = GL_ONE_MINUS_SRC_ALPHA;

        // Face Culling
        bool CullFace = false;
        GLenum CullMode = GL_BACK;

        // Stencil (optional später)
        bool StencilTest = false;

        // Vergleich (wichtig für State Change Minimierung)
        bool operator==(const RenderState& other) const {
            return DepthTest == other.DepthTest &&
                DepthWrite == other.DepthWrite &&
                DepthFunc == other.DepthFunc &&
                Blend == other.Blend &&
                BlendSrc == other.BlendSrc &&
                BlendDst == other.BlendDst &&
                CullFace == other.CullFace &&
                CullMode == other.CullMode &&
                StencilTest == other.StencilTest;
        }

        bool operator!=(const RenderState& other) const {
            return !(*this == other);
        }
    };

}