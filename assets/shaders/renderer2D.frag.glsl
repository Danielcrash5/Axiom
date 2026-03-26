#version 460 core

in vec4 v_Color;
in vec2 v_TexCoord;
flat in int v_TexIndex;

out vec4 FragColor;

// Array aus 32 Texturen
uniform sampler2D u_Textures[32];

void main()
{
    vec4 texColor = v_Color;

    // Nur multiplizieren, wenn eine Textur verwendet wird (Index > 0)
    if(v_TexIndex > 0)
        texColor *= texture(u_Textures[v_TexIndex], v_TexCoord);

    FragColor = texColor;
}