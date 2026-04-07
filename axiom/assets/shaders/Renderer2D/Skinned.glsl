#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_BoneIndices;
layout(location = 4) in vec4 a_BoneWeights;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat4 u_BoneTransforms[64];

out vec4 v_Color;
out vec2 v_TexCoord;

void main()
{
    mat4 skin =
        u_BoneTransforms[int(a_BoneIndices.x)] * a_BoneWeights.x +
        u_BoneTransforms[int(a_BoneIndices.y)] * a_BoneWeights.y +
        u_BoneTransforms[int(a_BoneIndices.z)] * a_BoneWeights.z +
        u_BoneTransforms[int(a_BoneIndices.w)] * a_BoneWeights.w;

    vec4 localPosition = skin * vec4(a_Position, 1.0);

    v_Color = a_Color;
    v_TexCoord = a_TexCoord;

    gl_Position = u_ViewProjection * u_Transform * localPosition;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
    FragColor = texture(u_Texture, v_TexCoord) * v_Color;
}
