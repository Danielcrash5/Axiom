#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 v_UV;

uniform vec4 u_Color;
uniform float u_Thickness;
uniform float u_Feather;
uniform float u_UseTexture;
uniform sampler2D u_Textures[32];


void main()
{
    float dist = length(v_UV - vec2(0.5));
    float radius = 0.5;

    float alpha;

    if (u_Thickness <= 0.0)
    {
        alpha = smoothstep(radius, radius - u_Feather, dist);
    }
    else
    {
        float inner = radius - u_Thickness;

        float outer = smoothstep(radius, radius - u_Feather, dist);
        float innerEdge = smoothstep(inner - u_Feather, inner, dist);

        alpha = outer * innerEdge;
    }

    FragColor = vec4(u_Color.rgb, u_Color.a * alpha);
}