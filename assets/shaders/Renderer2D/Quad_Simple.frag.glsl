#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 v_UV;
in vec4 v_Color;
flat in int v_TextureIndex;

uniform int u_UseTexture;
uniform int u_TextureSlot;
uniform sampler2D u_Textures[32];

void main()
{
    vec4 color = v_Color;
    
    if (u_UseTexture == 1 && v_TextureIndex >= 0) {
        color *= texture(u_Textures[v_TextureIndex], v_UV);
    }
    
    FragColor = color;
}
