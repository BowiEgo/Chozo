#version 410 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_IDBuffer;

in vec4 v_Color;
in vec2 v_TexCoord;
// in float v_TexIndex;
flat in int v_ObjectID;

// uniform sampler2D u_Textures[16];

void main()
{
    // int index = int(v_TexIndex);
    // o_Color = texture(u_Textures[index], v_TexCoord) * v_Color;
    o_Color = v_Color;
    // o_Color = vec4(v_TexCoord.xy, 1.0, 1.0);
    // o_Color = vec4(vec3(1.0), 1.0);
    o_IDBuffer = v_ObjectID;
}