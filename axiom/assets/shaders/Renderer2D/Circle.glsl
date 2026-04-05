#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in float a_Thickness;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;
out float v_Thickness;
out vec3 v_LocalPos;

void main()
{
    v_Color = a_Color;
    v_Thickness = a_Thickness;

    vec4 worldPos = u_Transform * vec4(a_Position, 1.0);
    v_LocalPos = a_Position;

    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in float v_Thickness;
in vec3 v_LocalPos;

void main()
{
    float dist = length(v_LocalPos.xy);
    float edge = 1.0 - v_Thickness;

    float alpha = smoothstep(1.0, 1.0 - edge, dist);

    FragColor = vec4(v_Color.rgb, v_Color.a * alpha);
}