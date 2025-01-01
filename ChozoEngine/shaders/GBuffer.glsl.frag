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

layout(binding = 0) uniform sampler2D u_NormalMap;
layout(binding = 1) uniform sampler2D u_BaseColorMap;
layout(binding = 2) uniform sampler2D u_MetallicRoughnessMap;
layout(binding = 3) uniform sampler2D u_OcclusionMap;
layout(binding = 4) uniform sampler2D u_EmissiveMap;

layout(push_constant) uniform PushConstants
{
    vec3 BaseColor;
    float OcclusionStrength;
    vec3 Emissive;
    float EmissiveStrength;
    float Metallic;
    float Roughness;

    int EnableBaseColorMap;
    int EnableMetallicRoughnessMap;
    int EnableNormalMap;
    int EnableOcclusionMap;
    int EnableEmissiveMap;

    int ID;
} u_Material;

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

    o_Position = v_FragPosition;
    o_Normal = (bool(u_Material.EnableNormalMap)) ? worldNormal : normalize(v_WorldNormal);
    o_Depth = vec3(depth);
    o_BaseColor = bool(u_Material.EnableBaseColorMap) ? baseColorMap.rgb : u_Material.BaseColor;
    o_MaterialProperties.r = bool(u_Material.EnableMetallicRoughnessMap) ? metallicRoughnessMap.b : u_Material.Metallic;
    o_MaterialProperties.g = bool(u_Material.EnableMetallicRoughnessMap) ? metallicRoughnessMap.g : u_Material.Roughness;
    o_MaterialProperties.b = bool(u_Material.EnableOcclusionMap) ? occlusionMap.r * u_Material.OcclusionStrength : 0.0;
    o_Emissive = vec4(bool(u_Material.EnableEmissiveMap) ? emissiveMap.rgb : u_Material.Emissive, u_Material.EmissiveStrength);
    o_EntityID = u_Material.ID;
}