#version 450

layout(location = 0) out vec4 o_Position;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Depth;
layout(location = 3) out vec4 o_Albedo;
layout(location = 4) out vec4 o_MaterialProperties;
layout(location = 5) out int o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;
layout(location = 3) in flat int v_EntityID;

layout(push_constant) uniform FragUniforms
{
    vec3 Albedo;
    float Metalness;
    float Roughness;
    float Ambient;
    float AmbientStrength;
    float Specular;
} u_Material;

layout(binding = 0) uniform sampler2D u_NormalTex;
layout(binding = 1) uniform sampler2D u_AlbedoTex;
layout(binding = 2) uniform sampler2D u_MetalnessTex;
layout(binding = 3) uniform sampler2D u_RoughnessTex;
layout(binding = 4) uniform sampler2D u_AmbientTex;
layout(binding = 5) uniform sampler2D u_SpecularTex;

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

    o_Position = vec4(v_FragPosition, 1.0);
    o_Normal = vec4(normalize(v_Normal), 1.0);
    o_Depth = vec4(vec3(depth), 1.0);
    o_Albedo = vec4(u_Material.Albedo, 1.0);
    o_MaterialProperties.r = u_Material.Metalness;
    o_MaterialProperties.g = u_Material.Roughness;
    o_MaterialProperties.b = u_Material.Ambient * u_Material.AmbientStrength;
    o_MaterialProperties.a = u_Material.Specular;
    o_EntityID = v_EntityID;
}