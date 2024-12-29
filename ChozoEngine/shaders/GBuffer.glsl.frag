#version 450

layout(location = 0) out vec3 o_Position;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_Depth;
layout(location = 3) out vec3 o_BaseColor;
layout(location = 4) out vec4 o_MaterialProperties;
layout(location = 5) out int o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;

layout(push_constant) uniform PushConstants
{
    vec3 BaseColor;
    float Metallic;
    float Roughness;
    float Reflectance;
    float Ambient;
    float AmbientStrength;

    int EnableBaseColorTex;
    int EnableMetallicTex;
    int EnableRoughnessTex;
    int EnableNormalTex;

    int ID;
} u_Material;

layout(binding = 0) uniform sampler2D u_NormalTex;
layout(binding = 1) uniform sampler2D u_BaseColorTex;
layout(binding = 2) uniform sampler2D u_MetallicTex;
layout(binding = 3) uniform sampler2D u_RoughnessTex;
layout(binding = 4) uniform sampler2D u_AmbientTex;

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

    o_Position = v_FragPosition;
    o_Normal = (u_Material.EnableNormalTex == 1) ? texture(u_NormalTex, v_TexCoord).rgb * 2.0 - vec3(1.0) : normalize(v_Normal);
    o_Depth = vec3(depth);
    o_BaseColor = (u_Material.EnableBaseColorTex == 1) ? texture(u_BaseColorTex, v_TexCoord).rgb : u_Material.BaseColor;
    o_MaterialProperties.r = (u_Material.EnableMetallicTex == 1) ? texture(u_MetallicTex, v_TexCoord).g : u_Material.Metallic;
    o_MaterialProperties.g = (u_Material.EnableRoughnessTex == 1) ? texture(u_RoughnessTex, v_TexCoord).r : u_Material.Roughness;
    o_MaterialProperties.b = u_Material.Reflectance;
    o_MaterialProperties.a = u_Material.Ambient * u_Material.AmbientStrength;
    o_EntityID = u_Material.ID;
}