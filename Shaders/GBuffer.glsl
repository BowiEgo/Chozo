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

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec3 v_WorldNormal;
layout(location = 2) out vec3 v_WorldTangent;
layout(location = 3) out vec3 v_WorldBitangent;
layout(location = 4) out vec2 v_TexCoord;
layout(location = 5) out vec3 v_FragPosition;

void main() {
    vec4 modelPosition = u_VertContant.ModelMatrix * vec4(a_Position, 1.0);
    vec4 viewPosition = u_Camera.ViewMatrix * modelPosition;
    vec4 projectionPosition = u_Camera.ProjMatrix * viewPosition;

    gl_Position = projectionPosition;

    mat3 normalMatrix = u_VertContant.NormalMatrix;
    v_TexCoord = a_TexCoord;
    v_FragPosition = vec3(viewPosition);

    v_Normal = a_Normal;
    v_WorldNormal = normalMatrix * a_Normal;
    v_WorldTangent = normalMatrix * a_Tangent;
    v_WorldBitangent = normalMatrix * a_Bitangent;
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_WorldNormal;
layout(location = 2) in vec3 v_WorldTangent;
layout(location = 3) in vec3 v_WorldBitangent;
layout(location = 4) in vec2 v_TexCoord;
layout(location = 5) in vec3 v_FragPosition;

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 BaseColor;
    vec4 Emissive;
    float Metallic;
    float Roughness;
    float AOIntensity;
    int UseAlbedoMap;
    int UseNormalMap;
    int UseRMAOMap;
    int UseEmissiveMap;
} u_Material;

layout(set = 1, binding = 1) uniform sampler2D u_AlbedoMap;
layout(set = 1, binding = 2) uniform sampler2D u_NormalMap;
layout(set = 1, binding = 3) uniform sampler2D u_RMAOMap;
layout(set = 1, binding = 4) uniform sampler2D u_EmissiveMap;

layout(location = 0) out vec3 o_Position;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_BaseColor;
layout(location = 3) out vec3 o_RMAO;
layout(location = 4) out vec3 o_Emissive;

#include "shaders://Utils/Math.glsl"
#include "shaders://Utils/Packing.glsl"

void main() {
    mat3 TBN;
    TBN[0] = normalize(v_WorldTangent);
    TBN[1] = normalize(v_WorldBitangent);
    TBN[2] = normalize(v_WorldNormal);

    TBN[1] = normalize(cross(TBN[2], TBN[0]));
    TBN[2] = normalize(cross(TBN[0], TBN[1]));

    vec4 normalMap = texture(u_NormalMap, v_TexCoord);
    vec4 albedoMap = texture(u_AlbedoMap, v_TexCoord);
    vec4 RMAOMap = texture(u_RMAOMap, v_TexCoord);
    vec4 emissiveMap = texture(u_EmissiveMap, v_TexCoord);

    vec3 perturbation = UnpackRGBToNormal(normalMap.rgb);
    vec3 perturbedNormal = normalize(TBN * perturbation);

    o_Position = v_FragPosition;
    o_Normal = (u_Material.UseNormalMap == 1) ? perturbedNormal : normalize(v_WorldNormal);

    o_BaseColor = u_Material.BaseColor.rgb;
    if (u_Material.UseAlbedoMap == 1)
        o_BaseColor *= albedoMap.rgb;

    o_RMAO.r = u_Material.Roughness;
    if (u_Material.UseRMAOMap == 1)
        o_RMAO.r *= RMAOMap.r;

    o_RMAO.g = u_Material.Metallic;
    if (u_Material.UseRMAOMap == 1)
        o_RMAO.g *= RMAOMap.g;

    o_RMAO.b = (u_Material.UseRMAOMap == 1) ? RMAOMap.b * u_Material.AOIntensity : 0.0;

    o_Emissive = u_Material.Emissive.rgb;
    if (u_Material.UseEmissiveMap == 1)
        o_Emissive.rgb *= emissiveMap.rgb;
}

#endif
