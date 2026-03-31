
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_TexIndex;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_UV;
out vec4 v_Color;
flat out int v_TexIndex;

void main()
{
    v_UV = a_UV;
    v_Color = a_Color;
    v_TexIndex = int(a_TexIndex);
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}
