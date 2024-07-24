#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
// layout(location = 3) in float a_TexIndex;
layout(location = 3) in int a_EntityID;

layout(std140) uniform Camera
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    // mat4 u_ProjectionViewMatrix;
};

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    // float TexIndex;
    // float TilingFactor;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) flat out int v_EntityID;

uniform mat4 u_ViewProjection;
uniform mat4 u_ModelMatrix;

out vec4 v_Color;
out vec2 v_TexCoord;
// out float v_TexIndex;
// flat out int v_EntityID;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    // v_TexIndex = a_TexIndex;

    Output.Color = a_Color;
    Output.TexCoord = a_TexCoord;
    // Output.TexIndex = a_TexIndex;
    // Output.TilingFactor = a_TilingFactor;

    v_EntityID = a_EntityID;
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
    // gl_Position = u_ProjectionViewMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
    // gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
}
