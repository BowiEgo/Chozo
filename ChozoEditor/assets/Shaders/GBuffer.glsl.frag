#version 450

layout(location = 0) out vec4 o_Position;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Depth;
layout(location = 3) out vec4 o_Ambient;
layout(location = 4) out vec4 o_Diffuse;
layout(location = 5) out vec4 o_Specular;
layout(location = 6) out vec4 o_Metalness;
layout(location = 7) out vec4 o_Roughness;
// layout(location = 8) out int o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;
layout(location = 3) in flat int v_EntityID;

layout(push_constant) uniform FragUniforms
{
    vec3 AmbientColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
    float AmbientStrength;
    float Metalness;
    float Roughness;
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
    float alpha = v_EntityID == -1 ? 0.0 : 1.0;
    float depth = LinearizeDepth(pow(gl_FragCoord.z, 0.3)) / far;

    o_Position = vec4(v_FragPosition, 1.0);
    o_Normal = vec4(normalize(v_Normal), 1.0);
    o_Depth = vec4(vec3(depth), 1.0);
    o_Ambient = vec4(u_Material.AmbientColor * u_Material.AmbientStrength, 1.0);
    o_Diffuse = vec4(u_Material.DiffuseColor, 1.0);
    o_Specular = vec4(u_Material.SpecularColor, 1.0);
    o_Metalness = vec4(vec3(u_Material.Metalness), 1.0);
    o_Roughness = vec4(vec3(u_Material.Roughness), 1.0);

    // o_EntityID = v_EntityID;
}