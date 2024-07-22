#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
// layout(location = 3) in float a_TexIndex;
layout(location = 3) in int a_ObjectID;

uniform mat4 u_ViewProjection;
uniform mat4 u_ModelMatrix;

out vec4 v_Color;
out vec2 v_TexCoord;
// out float v_TexIndex;
flat out int v_ObjectID;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    // v_TexIndex = a_TexIndex;
    v_ObjectID = a_ObjectID;
    gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
}
