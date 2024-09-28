#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(location = 0) out vec2 v_TexCoord;
layout(location = 1) out vec3 v_FragPosition;

void main()
{
    gl_Position = vec4(a_Position.xy, 0.0, 1.0);;

    v_TexCoord = a_TexCoord;
    v_FragPosition = a_Position;
}