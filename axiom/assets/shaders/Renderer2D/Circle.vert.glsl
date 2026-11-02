#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;
out float v_Thickness;
out vec3 v_LocalPos;

void main()
{
    v_Color = a_Color;
    v_Thickness = a_Thickness;
    v_LocalPos = a_LocalPosition;

    vec4 worldPos = u_Transform * vec4(a_Position, 1.0);
    gl_Position = u_ViewProjection * worldPos;
}
