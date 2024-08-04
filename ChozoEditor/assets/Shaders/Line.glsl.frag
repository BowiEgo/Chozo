#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
    vec4 Color;
};

layout(location = 0) in VertexOutput Vertex;
layout(location = 1) in flat int v_EntityID;

void main()
{
    o_Color = Vertex.Color;
    o_EntityID = v_EntityID;
}