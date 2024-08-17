#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    mat4 u_InverseViewProjectionMatrix;
};

struct VertexOutput
{
    vec4 Color;
};

layout(location = 0) out VertexOutput Vertex;
layout(location = 1) out flat int v_EntityID;

void main()
{
    Vertex.Color = a_Color;
    v_EntityID = a_EntityID;
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * vec4(a_Position, 1.0);
}