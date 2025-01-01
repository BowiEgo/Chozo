#version 450

layout(location = 0) out vec3 o_Tangent;
//layout(location = 1) out vec3 o_Bitangent;
//layout(location = 2) out vec3 o_NormalMap;
//layout(location = 3) out vec3 o_Occlusion;

//layout(location = 0) in vec3 v_Normal;
//layout(location = 1) in vec3 v_Tangent;
//layout(location = 2) in vec3 v_Bitangent;
//layout(location = 3) in vec2 v_TexCoord;
//layout(location = 4) in vec3 v_FragPosition;
//layout(location = 5) in vec3 v_WorldNormal;
//layout(location = 6) in vec3 v_WorldTangent;
//layout(location = 7) in vec3 v_WorldBitangent;

//layout(binding = 0) uniform sampler2D u_NormalMap;
//layout(binding = 1) uniform sampler2D u_BaseColorMap;
//layout(binding = 2) uniform sampler2D u_MetallicRoughnessMap;
//layout(binding = 3) uniform sampler2D u_OcclusionMap;
//layout(binding = 4) uniform sampler2D u_EmissiveMap;
//
//layout(push_constant) uniform PushConstants
//{
//    vec3 BaseColor;
//    float OcclusionStrength;
//    vec3 Emissive;
//    float EmissiveStrength;
//    float Metallic;
//    float Roughness;
//
//    int EnableBaseColorMap;
//    int EnableMetallicRoughnessMap;
//    int EnableNormalMap;
//    int EnableOcclusionMap;
//    int EnableEmissiveMap;
//
//    int ID;
//} u_Material;

void main()
{
//    vec4 normalMap = texture(u_NormalMap, v_TexCoord);
//    vec4 baseColorMap = texture(u_BaseColorMap, v_TexCoord);
//    vec4 metallicRoughnessMap = texture(u_MetallicRoughnessMap, v_TexCoord);
//    vec4 occlusionMap = texture(u_OcclusionMap, v_TexCoord);
//    vec4 emissiveMap = texture(u_EmissiveMap, v_TexCoord);

    o_Tangent = vec3(1.0);
//    o_Bitangent = v_Bitangent;
//    o_NormalMap = normalMap.rgb;
//    o_Occlusion = occlusionMap.rgb * u_Material.OcclusionStrength;
}