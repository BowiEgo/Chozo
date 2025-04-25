#version 450

layout(location = 0) out vec3 o_Position;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_Depth;
layout(location = 3) out vec3 o_BaseColor;
layout(location = 4) out vec3 o_MaterialProperties;
layout(location = 5) out vec4 o_Emissive;
layout(location = 6) out int o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;

layout(push_constant) uniform PushConstants
{
    vec4 BaseColor;
    vec4 Emissive;
    float Metallic;
    float Roughness;
    float Reflectance;
    float AmbientStrength;

    int EnableBaseColorMap;
    int EnableMetallicMap;
    int EnableRoughnessMap;
    int EnableAOMap;
    int EnableNormalMap;
    int EnableEmissiveMap;

    int ID;
} u_Material;

layout(binding = 0) uniform sampler2D u_NormalMap;
layout(binding = 1) uniform sampler2D u_BaseColorMap;
layout(binding = 2) uniform sampler2D u_MetallicRoughnessMap;
layout(binding = 3) uniform sampler2D u_AOMap;
layout(binding = 4) uniform sampler2D u_EmissiveMap;

float near = 0.1;
float far  = 20.0;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
    float depth = LinearizeDepth(pow(gl_FragCoord.z, 0.3)) / far;

    vec4 normalMap = texture(u_NormalMap, v_TexCoord);
    normalMap.g = 1.0 - normalMap.g;
    vec4 baseColorMap = texture(u_BaseColorMap, v_TexCoord);
    vec4 metallicRoughnessMap = texture(u_MetallicRoughnessMap, v_TexCoord);
    vec4 aoMap = texture(u_AOMap, v_TexCoord);
    vec4 emissiveMap = texture(u_EmissiveMap, v_TexCoord);

    o_Position = v_FragPosition;
    o_Normal = (u_Material.EnableNormalMap == 1) ? texture(u_NormalMap, v_TexCoord).rgb * 2.0 - vec3(1.0) : normalize(v_Normal);
    o_Depth = vec3(depth);

    o_BaseColor = u_Material.BaseColor.rgb * u_Material.BaseColor.a;
    if (u_Material.EnableBaseColorMap)
        o_BaseColor *= baseColorMap.rgb;

    o_MaterialProperties.r = u_Material.Metallic;
    if (u_Material.EnableMetallicMap)
        o_MaterialProperties.r *= metallicRoughnessMap.b;

    o_MaterialProperties.g = u_Material.Roughness;
    if (u_Material.EnableRoughnessMap)
        o_MaterialProperties.g *= metallicRoughnessMap.g;

    o_MaterialProperties.b = u_Material.EnableAOMap ? aoMap.r * u_Material.AmbientStrength : 0.0;

    o_Emissive = u_Material.Emissive;
    if (EnableEmissiveMap)
        o_Emissive.rgb *= emissiveMap.rgb;

    o_EntityID = u_Material.ID;
}