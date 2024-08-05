#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec3 a_Tangent;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
};

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat int v_EntityID;

void main()
{
    v_Normal = a_Normal;
    v_TexCoord = a_TexCoord;
    v_EntityID = a_EntityID;
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * vec4(a_Position, 1.0);
}