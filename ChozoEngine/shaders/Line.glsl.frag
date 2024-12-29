#version 450

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
    vec4 Color;
};

layout(location = 0) in VertexOutput Vertex;

void main()
{
    o_Color = Vertex.Color;
}