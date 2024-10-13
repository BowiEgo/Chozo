#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 0) in vec2 v_TexCoord;

layout(binding = 0) uniform isampler2D u_SolidIdTex;
layout(binding = 1) uniform isampler2D u_SolidDepthTex;
layout(binding = 2) uniform isampler2D u_PBRIdTex;
layout(binding = 3) uniform isampler2D u_PBRDepthTex;

vec3 getRandomColor(int id) {
    float r = float((id * 37) % 255) / 255.0;
    float g = float((id * 57) % 255) / 255.0;
    float b = float((id * 73) % 255) / 255.0;
    return vec3(r, g, b);
}

void main()
{
    int id = -1;
    float solidDepth = texture(u_SolidDepthTex, v_TexCoord).r;
    float PBRDepth = texture(u_PBRDepthTex, v_TexCoord).r;

    if (solidDepth > 0.0 && PBRDepth > 0.0)
    {
        if (solidDepth < PBRDepth)
        {
            id = texture(u_SolidIdTex, v_TexCoord).r;
        }
        else
        {
            id = texture(u_PBRIdTex, v_TexCoord).r;
        }
    }
    else if (solidDepth > 0.0)
    {
        id = texture(u_SolidIdTex, v_TexCoord).r;
    }
    else if (PBRDepth > 0.0)
    {
        id = texture(u_PBRIdTex, v_TexCoord).r;
    }

    o_Color = vec4(getRandomColor(id), 1.0);
    o_EntityID = id;
}