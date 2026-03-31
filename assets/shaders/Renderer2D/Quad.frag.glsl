#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 v_UV;
in vec4 v_Color;
flat in int v_TexIndex;

uniform vec4 u_Color;
uniform sampler2D u_Textures[16];

void main()
{
    vec4 color = v_Color * u_Color;

    if (v_TexIndex >= 0) {
        color *= texture(u_Textures[v_TexIndex], v_UV);
    }

    FragColor = color;
}
