#version 460 core

#extension GL_GOOGLE_include_directive : require
#include "Renderer2DCommon.glsl"

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in float v_Thickness;
in vec3 v_LocalPos;

void main()
{
    float thickness = clamp(v_Thickness, 0.0, 1.0);
    float dist = length(v_LocalPos.xy);
    float outerAlpha = 1.0 - smoothstep(0.98, 1.0, dist);
    float alpha = outerAlpha;

    if (thickness > 0.0) {
        float innerRadius = max(0.0, 1.0 - thickness);
        float innerMask = 1.0 - smoothstep(innerRadius - 0.02, innerRadius + 0.02, dist);
        alpha *= (1.0 - innerMask);
    }

    if (alpha <= 0.001)
        discard;

    FragColor = vec4(v_Color.rgb, v_Color.a * alpha);
}
