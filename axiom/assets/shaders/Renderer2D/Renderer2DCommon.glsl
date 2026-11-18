/**
 * @file axiom/assets/shaders/Renderer2D/Renderer2DCommon.glsl
 * @brief Defines shared shader utilities for the 2D renderer.
 */

#ifndef AXIOM_RENDERER2D_COMMON_GLSL
#define AXIOM_RENDERER2D_COMMON_GLSL

mat4 axiom_make_scale(float x, float y, float z) {
    return mat4(
        vec4(x, 0.0, 0.0, 0.0),
        vec4(0.0, y, 0.0, 0.0),
        vec4(0.0, 0.0, z, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0));
}

#endif
