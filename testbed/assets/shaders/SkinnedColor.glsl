#type vertex
#version 460 core

#include "VertexFormat.glsl"

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat4 u_BoneTransforms[64];

out vec4 v_Color;

void main()
{
    mat4 skin =
        u_BoneTransforms[int(a_BoneIndices.x)] * a_BoneWeights.x +
        u_BoneTransforms[int(a_BoneIndices.y)] * a_BoneWeights.y +
        u_BoneTransforms[int(a_BoneIndices.z)] * a_BoneWeights.z +
        u_BoneTransforms[int(a_BoneIndices.w)] * a_BoneWeights.w;

    vec4 localPosition = skin * vec4(a_Position, 1.0);
    v_Color = a_Color;
    gl_Position = u_ViewProjection * u_Transform * localPosition;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;
in vec4 v_Color;

void main()
{
    FragColor = v_Color;
}