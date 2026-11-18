/**
 * @file axiom/assets/shaders/Renderer2D/Quad.frag.glsl
 * @brief Shades 2D quads and samples their textures.
 */

#version 450

layout(location = 0) in vec4 v_Color;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = v_Color;
}
