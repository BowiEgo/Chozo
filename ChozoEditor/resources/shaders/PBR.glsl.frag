#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

layout(binding = 0) uniform sampler2D u_PositionTex;
layout(binding = 1) uniform sampler2D u_NormalTex;
layout(binding = 2) uniform sampler2D u_AlbedoTex;
layout(binding = 3) uniform sampler2D u_MaterialPropTex;
layout(binding = 4) uniform samplerCube u_IrradianceMap;
layout(binding = 5) uniform samplerCube u_PrefilterMap;
layout(binding = 6) uniform sampler2D u_BRDFLutTex;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
} m_Params;

#include "includes/PBRAlgorithms.glsl"
#include "includes/Scene.glsl"
#include "includes/PBRLight.glsl"

// ----------------------------------------------------------------------------
vec3 IBL(vec3 F0, vec3 Lr)
{
    vec3 irradiance = texture(u_IrradianceMap, m_Params.Normal).rgb;
	vec3 F = FresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
	vec3 diffuseIBL = m_Params.Albedo * irradiance;

    const float MAX_REFLECTION_LOD = 4.0;
	float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
    vec3 specularIrradiance = textureLod(u_PrefilterMap, R,  m_Params.Roughness * MAX_REFLECTION_LOD).rgb;    
	//specularIrradiance = vec3(Convert_sRGB_FromLinear(specularIrradiance.r), Convert_sRGB_FromLinear(specularIrradiance.g), Convert_sRGB_FromLinear(specularIrradiance.b));

	// Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
    vec2 specularBRDF  = texture(u_BRDFLutTex, vec2(m_Params.NdotV, m_Params.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

void main()
{
    vec3 gPosition = texture(u_PositionTex, v_TexCoord).rgb;

    float ao = 1.0;

    vec4 materialProps = texture(u_MaterialPropTex, v_TexCoord);

    m_Params.Albedo    = texture(u_AlbedoTex, v_TexCoord).rgb;
    m_Params.Metalness  = materialProps.r;
    m_Params.Roughness = max(materialProps.g, 0.001);
    m_Params.Normal = normalize(texture(u_NormalTex, v_TexCoord).rgb);
    m_Params.View = normalize(u_Scene.CameraPosition - gPosition);
    m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);
    // ao        = texture(aoMap, v_TexCoord).r;

    vec3 R = reflect(-m_Params.View, m_Params.Normal);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, m_Params.Albedo, m_Params.Metalness);

    // reflectance equation
    vec3 dirLights = CalculateDirLights(F0);
    vec3 pointLights = CalculatePointLights(F0, gPosition);

    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.1) * m_Params.Albedo * ao;

	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

    ambient = IBL(F0, Lr) * ao;

    vec3 color = ambient + dirLights + pointLights;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    float alpha = 1.0;
    if (m_Params.Normal == vec3(0.0))
        discard;
    o_Color = vec4(color, alpha);
}