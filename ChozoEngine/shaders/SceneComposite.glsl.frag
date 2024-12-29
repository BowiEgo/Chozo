#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

layout(binding = 0) uniform sampler2D u_SkyboxTex;
layout(binding = 1) uniform sampler2D u_SolidTex;
layout(binding = 2) uniform sampler2D u_SolidDepthTex;
layout(binding = 3) uniform sampler2D u_PBRTex;
layout(binding = 4) uniform sampler2D u_PBRDepthTex;

void main()
{
    vec3 color = vec3(0.0);
    float alpha = 1.0;
    float depth = 0.0;
    float solidDepth = texture(u_SolidDepthTex, v_TexCoord).r;
    float PBRDepth = texture(u_PBRDepthTex, v_TexCoord).r;

    vec4 envLayer = texture(u_SkyboxTex, v_TexCoord);
    vec4 solidLayer = texture(u_SolidTex, v_TexCoord);
    vec4 PBRLayer = texture(u_PBRTex, v_TexCoord);

    if (solidDepth > 0.0 || PBRDepth > 0.0)
    {
        if (solidDepth > 0.0 && PBRDepth > 0.0)
        {
            color = (solidDepth < PBRDepth) ? solidLayer.rgb : PBRLayer.rgb;
        }
        else if (solidDepth > 0.0)
        {
            color = solidLayer.rgb;
        }
        else
        {
            color = PBRLayer.rgb;
        }
    }
    else
    {
//        color = envLayer.rgb;
        discard;
    }

    o_Color = vec4(color, alpha);
}
