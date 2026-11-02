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
