#version 450

layout(location = 0) out vec3 o_Position;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_Depth;
layout(location = 3) out vec3 o_BaseColor;
layout(location = 4) out vec3 o_MaterialProperties;
layout(location = 5) out vec4 o_Emissive;
layout(location = 6) out int o_EntityID;

layout(location = 0) in vec3 v_WorldNormal;
layout(location = 1) in vec3 v_WorldTangent;
layout(location = 2) in vec3 v_WorldBitangent;
layout(location = 3) in vec2 v_TexCoord;
layout(location = 4) in vec3 v_FragPosition;

const uint BASE_COLOR_MAP_FLAG = 0x01;
const uint METALLIC_MAP_FLAG   = 0x02;
const uint ROUGHNESS_MAP_FLAG  = 0x04;
const uint NORMAL_MAP_FLAG     = 0x08;
const uint EMISSIVE_MAP_FLAG   = 0x10;
const uint OCCLUSION_MAP_FLAG  = 0x20;

struct MaterialParams
{
    vec4 BaseColor;             // Base color (16 bytes)
    vec4 Emissive;              // Emissive.rgb and EmissiveIntensity (16 bytes)
    vec3 MiscParams;            // Metallic, Roughness, OcclusionIntensity (12 bytes)
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
layout(binding = 3) uniform sampler2D u_OcclusionMap;
layout(binding = 4) uniform sampler2D u_EmissiveMap;

bool IsFlagEnabled(uint flags, uint mask) {
    return (flags & mask) != 0;
}

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

    float depth = LinearizeDepth(pow(gl_FragCoord.z, 0.3)) / far;

    vec4 normalMap = texture(u_NormalMap, v_TexCoord);
    vec4 baseColorMap = texture(u_BaseColorMap, v_TexCoord);
    vec4 metallicRoughnessMap = texture(u_MetallicRoughnessMap, v_TexCoord);
    vec4 occlusionMap = texture(u_OcclusionMap, v_TexCoord);
    vec4 emissiveMap = texture(u_EmissiveMap, v_TexCoord);

    vec3 tangentNormal = normalMap.rgb * 2.0 - 1.0;
    vec3 worldNormal = normalize(TBN * tangentNormal);

    MaterialParams matParams;
    matParams = u_MaterialParams.Materials[u_Constant.MaterialIndex];

    bool EnableBaseColorMap = IsFlagEnabled(matParams.Flags, BASE_COLOR_MAP_FLAG);
    bool EnableMetallicMap = IsFlagEnabled(matParams.Flags, METALLIC_MAP_FLAG);
    bool EnableRoughnessMap = IsFlagEnabled(matParams.Flags, ROUGHNESS_MAP_FLAG);
    bool EnableNormalMap = IsFlagEnabled(matParams.Flags, NORMAL_MAP_FLAG);
    bool EnableEmissiveMap = IsFlagEnabled(matParams.Flags, EMISSIVE_MAP_FLAG);
    bool EnableOcclusionMap = IsFlagEnabled(matParams.Flags, OCCLUSION_MAP_FLAG);

    o_Position = v_FragPosition;
    o_Normal = EnableNormalMap ? worldNormal : normalize(v_WorldNormal);
    o_Depth = vec3(depth);
    o_BaseColor = EnableBaseColorMap ? baseColorMap.rgb : matParams.BaseColor.rgb;
    o_MaterialProperties.r = EnableMetallicMap ? metallicRoughnessMap.b : matParams.MiscParams.r;
    o_MaterialProperties.g = EnableRoughnessMap ? metallicRoughnessMap.g : matParams.MiscParams.g;
    o_MaterialProperties.b = EnableOcclusionMap ? occlusionMap.r * matParams.MiscParams.b : 0.0;
    o_Emissive = EnableEmissiveMap ? vec4(emissiveMap.rgb, matParams.Emissive.a) : matParams.Emissive;
    o_EntityID = u_Constant.ID;
}