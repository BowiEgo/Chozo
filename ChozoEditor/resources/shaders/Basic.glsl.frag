#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;
layout(location = 3) in flat int v_EntityID;

layout(push_constant) uniform Uniforms
{
    vec4 Color;
} u_FragUniforms;

void main()
{
    vec4 texColor = u_FragUniforms.Color;

    o_Color = texColor;
    o_EntityID = v_EntityID;
}