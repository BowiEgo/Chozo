#version 460

#ifdef VERTEX_SHADER

// ===== Uniform =====
#include "shaders://Camera.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(push_constant) uniform VertexContant
{
    mat4 ModelMatrix;
    mat3 NormalMatrix;
} u_VertContant;

layout(location = 0) out vec2 v_TexCoord;
layout(location = 1) out vec3 v_Normal;

void main() {
    mat4 model = u_VertContant.ModelMatrix;
    mat4 mvp = u_Camera.ProjMatrix * u_Camera.ViewMatrix * model;
    
    gl_Position = mvp * vec4(a_Position, 1.0);

    float dummy = a_Tangent.x + a_Bitangent.x;

    v_TexCoord = a_TexCoord;
    v_Normal = normalize(u_VertContant.NormalMatrix * a_Normal);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_Normal;

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 u_BaseColor;
    float u_Metallic;
    float u_Roughness;
    float u_NormalStrength;
    float u_EmissiveStrength;
    int UseAlbedoMap;
    int UseNormalMap;
    int UseRMAOMap;
} u_Material;

layout(set = 1, binding = 1) uniform sampler2D u_AlbedoMap;
layout(set = 1, binding = 2) uniform sampler2D u_NormalMap;
layout(set = 1, binding = 3) uniform sampler2D u_RMAOMap;

void main() {
    vec4 albedo = u_Material.u_BaseColor;
    float metallic = u_Material.u_Metallic;
    float roughness = u_Material.u_Roughness;

    vec3 normal = v_Normal;

    vec4 rmao = texture(u_RMAOMap, v_TexCoord);

    if (u_Material.UseAlbedoMap != 0) {
        albedo *= texture(u_AlbedoMap, v_TexCoord);
    }
    if (u_Material.UseNormalMap != 0) {
        normal = texture(u_NormalMap, v_TexCoord).rgb;
    }
    if (u_Material.UseRMAOMap != 0) {
        roughness *= rmao.r;
    }
    if (u_Material.UseRMAOMap != 0) {
        metallic *= rmao.g;
    }

    vec3 N = normalize(v_Normal);
    vec3 L = normalize(vec3(1.0, 1.0, 0.0));
    float diff = max(dot(N, L), 0.0);
    vec3 color = albedo.rgb * diff;

    // o_Color = vec4(color, 1.0);
    o_Color = albedo;
}

#endif
