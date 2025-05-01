#version 450

layout(location = 0) out vec3 o_Position;
layout(location = 1) out vec3 o_PerturbedNormal;
layout(location = 2) out vec3 o_Depth;
layout(location = 3) out vec3 o_BaseColor;
layout(location = 4) out vec3 o_MetallicRoughnessAO;
layout(location = 5) out vec4 o_Emissive;
layout(location = 6) out int o_EntityID;

#include "Snippets/Fragment/ModelVaryings.glsl"

layout(push_constant) uniform FragUniforms
{
    vec3 BaseColor;
    float EmissiveIntensity;
    vec3 Emissive;
    float Metallic;
    float Roughness;
    float AOIntensity;

    int EnableBaseColorMap;
    int EnableMetallicMap;
    int EnableRoughnessMap;
    int EnableAOMap;
    int EnableNormalMap;
    int EnableEmissiveMap;
    int ID;
} u_Constant;

layout(binding = 0) uniform sampler2D u_NormalMap;
layout(binding = 1) uniform sampler2D u_BaseColorMap;
layout(binding = 2) uniform sampler2D u_MetallicRoughnessMap;
layout(binding = 3) uniform sampler2D u_AOMap;
layout(binding = 4) uniform sampler2D u_EmissiveMap;

#include "Includes/Math.glsl"
#include "Includes/Packing.glsl"

float near = 0.1;
float far  = 20.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    mat3 TBN;
    TBN[0] = normalize(v_WorldTangent);
    TBN[1] = normalize(v_WorldBitangent);
    TBN[2] = normalize(v_WorldNormal);

    TBN[1] = normalize(cross(TBN[2], TBN[0]));
    TBN[2] = normalize(cross(TBN[0], TBN[1]));

    float near = 0.1;
    float far  = 20.0;
    float depthNDC = Saturate(gl_FragCoord.z);
    float viewZ = LinearizeNDCDepthToViewZ(depthNDC, near, far);
    float normalizedDepth = viewZ / far;
    float visualizedDepth = pow(normalizedDepth, 0.4);

    vec4 normalMap = texture(u_NormalMap, v_TexCoord);
    normalMap.g = 1.0 - normalMap.g;
    vec4 baseColorMap = texture(u_BaseColorMap, v_TexCoord);
    vec4 metallicRoughnessMap = texture(u_MetallicRoughnessMap, v_TexCoord);
    vec4 aoMap = texture(u_AOMap, v_TexCoord);
    vec4 emissiveMap = texture(u_EmissiveMap, v_TexCoord);

    vec3 perturbation = UnpackRGBToNormal(normalMap.rgb);
    vec3 perturbedNormal = normalize(TBN * perturbation);

    o_Position = v_FragPosition;
//    o_PerturbedNormal = (u_Constant.EnableNormalMap == 1) ? perturbedNormal : normalize(v_WorldNormal);
    o_PerturbedNormal = (u_Constant.EnableNormalMap == 1) ? texture(u_NormalMap, v_TexCoord).rgb * 2.0 - vec3(1.0) : normalize(v_Normal);
    o_Depth = vec3(visualizedDepth);

    o_BaseColor = u_Constant.BaseColor;
    if (u_Constant.EnableBaseColorMap == 1)
        o_BaseColor *= baseColorMap.rgb;

    o_MetallicRoughnessAO.r = u_Constant.Metallic;
    if (u_Constant.EnableMetallicMap == 1)
        o_MetallicRoughnessAO.r *= metallicRoughnessMap.b;

    o_MetallicRoughnessAO.g = u_Constant.Roughness;
    if (u_Constant.EnableRoughnessMap == 1)
        o_MetallicRoughnessAO.g *= metallicRoughnessMap.g;

    o_MetallicRoughnessAO.b = (u_Constant.EnableAOMap == 1) ? aoMap.r * u_Constant.AOIntensity : 0.0;

    o_Emissive = vec4(u_Constant.Emissive.rgb, u_Constant.EmissiveIntensity);
    if (u_Constant.EnableEmissiveMap == 1)
        o_Emissive.rgb *= emissiveMap.rgb;

    o_EntityID = u_Constant.ID;
}