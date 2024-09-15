#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

layout(binding = 0) uniform sampler2D u_SkyboxTex;
layout(binding = 1) uniform sampler2D u_GeometryTex;

void main()
{
    vec3 skyboxColor = texture(u_SkyboxTex, v_TexCoord).rgb;
    vec3 geometryColor = texture(u_GeometryTex, v_TexCoord).rgb;

    o_Color = mix(vec4(geometryColor, 1.0), vec4(skyboxColor, 1.0), 0.5);
}
