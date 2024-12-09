#version 450

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    float TilingFactor;
};

layout(location = 0) in VertexOutput Output;
layout(location = 3) in flat float v_TexIndex;
layout(location = 4) in flat int v_EntityID;

layout(binding = 0) uniform sampler2D u_Textures[MAX_TEXTURE_SLOTS];

void main()
{
    vec4 texColor = Output.Color;
    int idx = int(v_TexIndex);
    if (idx < MAX_TEXTURE_SLOTS)
        texColor *= texture(u_Textures[idx], Output.TexCoord * Output.TilingFactor);

    o_Color = texColor;
}