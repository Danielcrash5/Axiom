/**
 * @file axiom/assets/shaders/Renderer2D/Line.frag.glsl
 * @brief Applies color to rendered 2D lines.
 */

#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;

void main()
{
    FragColor = v_Color;
}
