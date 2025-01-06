#version 450

layout(location = 0) out vec3 o_Position;
layout(location = 1) out vec3 o_PerturbedNormal;
layout(location = 2) out vec3 o_Depth;
layout(location = 3) out vec3 o_BaseColor;
layout(location = 4) out vec3 o_MaterialProperties;
layout(location = 5) out vec4 o_Emissive;
layout(location = 6) out int o_EntityID;

#include "Snippets/Fragment/ModelVaryings.glsl"

const uint BASE_COLOR_MAP_FLAG = 0x01;
const uint METALLIC_MAP_FLAG   = 0x02;
const uint ROUGHNESS_MAP_FLAG  = 0x04;
const uint NORMAL_MAP_FLAG     = 0x08;
const uint EMISSIVE_MAP_FLAG   = 0x10;
const uint AO_MAP_FLAG  = 0x20;

struct MaterialParams
{
    vec4 BaseColor;             // Base color (16 bytes)
    vec4 Emissive;              // Emissive.rgb and EmissiveIntensity (16 bytes)
    vec3 MiscParams;            // Metallic, Roughness, AOIntensity (12 bytes)
    uint Flags;                 // Bitmask for all Enable* flags (4 bytes)
};

layout(std140, binding = 1) uniform MaterialParamsData
{
    uint MaterialCount;
    MaterialParams Materials[1000];
} u_MaterialParams;

layout(push_constant) uniform PushConstants
{
    int MaterialIndex;
    int ID;
} u_Constant;

layout(binding = 0) uniform sampler2D u_NormalMap;
layout(binding = 1) uniform sampler2D u_BaseColorMap;
layout(binding = 2) uniform sampler2D u_MetallicRoughnessMap;
layout(binding = 3) uniform sampler2D u_AOMap;
layout(binding = 4) uniform sampler2D u_EmissiveMap;

#include "Includes/Math.glsl"
#include "Includes/Packing.glsl"

bool IsFlagEnabled(uint flags, uint mask) {
    return (flags & mask) != 0;
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

    MaterialParams matParams;
    matParams = u_MaterialParams.Materials[u_Constant.MaterialIndex];

    bool EnableBaseColorMap = IsFlagEnabled(matParams.Flags, BASE_COLOR_MAP_FLAG);
    bool EnableMetallicMap = IsFlagEnabled(matParams.Flags, METALLIC_MAP_FLAG);
    bool EnableRoughnessMap = IsFlagEnabled(matParams.Flags, ROUGHNESS_MAP_FLAG);
    bool EnableNormalMap = IsFlagEnabled(matParams.Flags, NORMAL_MAP_FLAG);
    bool EnableEmissiveMap = IsFlagEnabled(matParams.Flags, EMISSIVE_MAP_FLAG);
    bool EnableAOMap = IsFlagEnabled(matParams.Flags, AO_MAP_FLAG);

    o_Position = v_FragPosition;
    o_PerturbedNormal = EnableNormalMap ? perturbedNormal : normalize(v_WorldNormal);
//    o_PerturbedNormal = normalize(v_WorldNormal);
    o_Depth = vec3(visualizedDepth);

    o_BaseColor = matParams.BaseColor.rgb;
    if (EnableBaseColorMap)
        o_BaseColor *= baseColorMap.rgb;

    o_MaterialProperties.r = matParams.MiscParams.r;
    if (EnableBaseColorMap)
        o_MaterialProperties.r *= metallicRoughnessMap.b;

    o_MaterialProperties.g = matParams.MiscParams.g;
    if (EnableRoughnessMap)
        o_MaterialProperties.g *= metallicRoughnessMap.g;

    o_MaterialProperties.b = EnableAOMap ? aoMap.r * matParams.MiscParams.b : 0.0;

    o_Emissive = matParams.Emissive;
    if (EnableEmissiveMap)
        o_Emissive.rgb *= emissiveMap.rgb;

    o_EntityID = u_Constant.ID;
}