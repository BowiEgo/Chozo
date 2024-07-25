#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
};

layout(location = 0) in VertexOutput Output;
layout(location = 3) in flat int v_EntityID;

void main()
{
    EntityID = v_EntityID;
    FragColor = Output.Color;
}