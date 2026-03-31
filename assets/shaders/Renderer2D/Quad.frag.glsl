#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 v_UV;
in vec4 v_Color;
flat in int v_TexIndex;

uniform vec4 u_Color;
uniform int u_UseTexture;
uniform sampler2D u_Textures[32];

void main()
{
    vec4 color = v_Color * u_Color;

    if (u_UseTexture == 1 && v_TexIndex >= 0) {
        color *= texture(u_Textures[v_TexIndex], v_UV);
    }

    FragColor = color;
}
