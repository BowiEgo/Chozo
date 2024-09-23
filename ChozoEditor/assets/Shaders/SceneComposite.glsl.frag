#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

layout(binding = 0) uniform sampler2D u_SkyboxTex;
layout(binding = 1) uniform sampler2D u_GeometryTex;
layout(binding = 2) uniform sampler2D u_DepthTex;

void main()
{
    vec3 color = vec3(0.0);
    float alpha = 1.0;
    float depth = 0.0;
    depth = texture(u_DepthTex, v_TexCoord).r;

    if (depth > 0.0)
        color = texture(u_GeometryTex, v_TexCoord).rgb;
    else
    {
        vec4 env = texture(u_SkyboxTex, v_TexCoord);
        color = env.rgb;
        if (env.a == 0)
            alpha = 0.0;
    }

    o_Color = vec4(color, alpha);
}
