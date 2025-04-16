#include "../Snippets/Fragment/Scene.glsl"

precision mediump int;
precision highp float;

layout(binding = 0) uniform sampler2D u_PositionMap;
layout(binding = 1) uniform sampler2D u_NormalMap;
layout(binding = 2) uniform sampler2D u_BaseColorMap;
layout(binding = 3) uniform sampler2D u_MaterialPropMap;
layout(binding = 4) uniform sampler2D u_EmissiveMap;

struct GBufferData
{
    vec3 BaseColor;
    float PerceptualRoughness;
    float Roughness;
    float Metallic;
    float Reflectance;
    vec3 Emissive;

    float AO;
    highp vec3 Position;
    vec3 PerturbedNormal;
    vec3 View;
    vec3 Reflected;
} GBuffer;

void InitGBuffer(out GBufferData GBuffer)
{
    bool isOrthographic = false;
    vec4 materialPropsTex = texture(u_MaterialPropMap, v_TexCoord);

    GBuffer.BaseColor           = texture(u_BaseColorMap, v_TexCoord).rgb;
    GBuffer.Metallic            = materialPropsTex.r;
    GBuffer.PerceptualRoughness = max(materialPropsTex.g, 0.001);
    GBuffer.Roughness           = GBuffer.PerceptualRoughness * GBuffer.PerceptualRoughness;
    GBuffer.Reflectance         = 0.4;
    GBuffer.Emissive            = texture(u_EmissiveMap, v_TexCoord).rgb;

//    GBuffer.EnergyCompensation = vec3(1.0);
    //    GBuffer.EnergyCompensation = 1.0 + f0 * (1.0 / dfg.y - 1.0);

    GBuffer.AO              = 1.0;
    GBuffer.Position        = texture(u_PositionMap, v_TexCoord).rgb;
    GBuffer.PerturbedNormal = normalize(texture(u_NormalMap, v_TexCoord).rgb);
    GBuffer.View            = (isOrthographic) ? vec3( 0, 0, 1 ) : normalize(-GBuffer.Position);
//    GBuffer.Reflected = reflect(-GBuffer.View, GBuffer.PerturbedNormal);
    GBuffer.Reflected       = 2.0 * dot(GBuffer.View, GBuffer.PerturbedNormal) * GBuffer.PerturbedNormal - GBuffer.View;
}