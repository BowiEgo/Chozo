#include "Math.glsl"
#include "Material.glsl"
#include "LightInfo.glsl"

layout(binding = 4) uniform samplerCube u_IrradianceMap;
layout(binding = 5) uniform samplerCube u_PrefilterMap;
layout(binding = 6) uniform sampler2D u_BRDFLutTex;

struct LightContext
{
    vec3 ClearcoatSpecularDirect;
    vec3 ClearcoatSpecularIndirect;
    vec3 SheenSpecularDirect;
    vec3 SheenSpecularIndirect;
};

vec3 BRDF_Lambert(const in vec3 diffuseColor)
{
    return RECIPROCAL_PI * diffuseColor;
}

// More reallistic but more cost
float V_GGX_SmithCorrelated_Heitz(const in float alpha, const in float NoL, const in float NoV)
{
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = NoL * sqrt((NoV - alpha * NoV) * NoV + alpha);
    float lambdaL = NoV * sqrt((NoL - alpha * NoL) * NoL + alpha);
    float v = 0.5 / (lambdaV + lambdaL);
    // a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
    // a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
    // clamp to the maximum value representable in mediump
    return saturateMediump(v);
}

// Moving Frostbite to Physically Based Rendering 3.0 - page 12, listing 2
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
float V_GGX_SmithCorrelated(const in float alpha, const in float NoL, const in float NoV)
{
    float a2 = pow2(alpha);

    float gv = NoL * sqrt(a2 + (1.0 - a2) * pow2(NoV));
    float gl = NoV * sqrt(a2 + (1.0 - a2) * pow2(NoL));

    return 0.5 / max(gv + gl, EPSILON);
}

vec3 F_Schlick(const in vec3 f0, const in float f90, const in float VoH)
{
    // Original approximation by Christophe Schlick '94
    // float fresnel = pow(1.0 - VoH, 5.0);

    // Optimized variant (presented by Epic at SIGGRAPH '13)
    // https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
    float fresnel = exp2((- 5.55473 * VoH - 6.98316) * VoH);

    return f0 * (1.0 - fresnel) + (f90 * fresnel);
}

float F_Schlick(const in float f0, const in float f90, const in float VoH)
{
    // Original approximation by Christophe Schlick '94
    // float fresnel = pow(1.0 - VoH, 5.0);

    // Optimized variant (presented by Epic at SIGGRAPH '13)
    // https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
    float fresnel = exp2((- 5.55473 * VoH - 6.98316) * VoH);

    return f0 * (1.0 - fresnel) + (f90 * fresnel);
}

// Microfacet Models for Refraction through Rough Surfaces - equation (33)
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// alpha is "roughness squared" in Disney’s reparameterization
float D_GGX(const in float alpha, const in float NoH)
{
    float a2 = pow2(alpha);

    float denom = pow2(NoH) * (a2 - 1.0) + 1.0; // avoid alpha = 0 with NoH = 1

    return RECIPROCAL_PI * a2 / pow2(denom);
}

// GGX Distribution, Schlick Fresnel, GGX_SmithCorrelated Visibility
vec3 BRDF_GGX_Clearcoat(const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in PhysicalMaterial material)
{
    vec3 f0 = material.ClearcoatF0;
    float f90 = material.ClearcoatF90;
    float roughness = material.ClearcoatRoughness;

    float alpha = pow2(roughness); // UE4's roughness

    vec3 halfDir = normalize(lightDir + viewDir);

    float NoL = Saturate(dot(normal, lightDir));
    float NoV = Saturate(dot(normal, viewDir));
    float NoH = Saturate(dot(normal, halfDir));
    float VoH = Saturate(dot(viewDir, halfDir));

    vec3 F = F_Schlick(f0, f90, VoH);
    float V = V_GGX_SmithCorrelated(alpha, NoL, NoV);
    float D = D_GGX(alpha, NoH);

    return F * (V * D);
}

// https://github.com/google/filament/blob/master/shaders/src/brdf.fs
float D_Charlie(float roughness, float NoH)
{
    float alpha = pow2(roughness);

    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
    float invAlpha = 1.0 / alpha;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16

    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

// https://github.com/google/filament/blob/master/shaders/src/brdf.fs
float V_Neubelt(float NoV, float NoL)
{
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return Saturate(1.0 / (4.0 * (NoL + NoV - NoL * NoV)));
}

vec3 BRDF_Sheen(const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, vec3 sheenColor, const in float sheenRoughness)
{
    vec3 halfDir = normalize(lightDir + viewDir);

    float NoL = Saturate(dot(normal, lightDir));
    float NoV = Saturate(dot(normal, viewDir));
    float NoH = Saturate(dot(normal, halfDir));

    float D = D_Charlie(sheenRoughness, NoH);
    float V = V_Neubelt(NoV, NoL);

    return sheenColor * (D * V);
}

float V_GGX_SmithCorrelated_Anisotropic(const in float alphaT, const in float alphaB, const in float dotTV, const in float dotBV, const in float dotTL, const in float dotBL, const in float NoV, const in float NoL)
{
    float gv = NoL * length(vec3(alphaT * dotTV, alphaB * dotBV, NoV));
    float gl = NoV * length(vec3(alphaT * dotTL, alphaB * dotBL, NoL));
    float v = 0.5 / (gv + gl);

    return Saturate(v);
}

float D_GGX_Anisotropic(const in float alphaT, const in float alphaB, const in float NoH, const in float dotTH, const in float dotBH)
{
    float a2 = alphaT * alphaB;
    highp vec3 v = vec3(alphaB * dotTH, alphaT * dotBH, a2 * NoH);
    highp float v2 = dot(v, v);
    float w2 = a2 / v2;

    return RECIPROCAL_PI * a2 * pow2 (w2);
}

vec3 BRDF_GGX(const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in PhysicalMaterial material)
{
    vec3 f0 = material.SpecularColor;
    float f90 = material.SpecularF90;
    float roughness = material.Roughness;

    float alpha = pow2(roughness); // UE4's roughness

    vec3 halfDir = normalize(lightDir + viewDir);

    float NoL = Saturate(dot(normal, lightDir));
    float NoV = Saturate(dot(normal, viewDir));
    float NoH = Saturate(dot(normal, halfDir));
    float VoH = Saturate(dot(viewDir, halfDir));

    vec3 F = F_Schlick(f0, f90, VoH);
#ifdef USE_IRIDESCENCE
    F = mix(F, material.IridescenceFresnel, material.Iridescence);
#endif

#ifdef USE_ANISOTROPY
    float dotTL = dot(material.AnisotropyT, lightDir);
    float dotTV = dot(material.AnisotropyT, viewDir);
    float dotTH = dot(material.AnisotropyT, halfDir);
    float dotBL = dot(material.AnisotropyB, lightDir);
    float dotBV = dot(material.AnisotropyB, viewDir);
    float dotBH = dot(material.AnisotropyB, halfDir);

    float V = V_GGX_SmithCorrelated_Anisotropic(material.AlphaT, alpha, dotTV, dotBV, dotTL, dotBL, NoV, NoL);
    float D = D_GGX_Anisotropic(material.AlphaT, alpha, NoH, dotTH, dotBH);
#else
    float V = V_GGX_SmithCorrelated(alpha, NoL, NoV);
    float D = D_GGX(alpha, NoH);
#endif

    return F * (V * D);
}

void RE_Direct_Physical(const in IncidentLight directLight, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in PhysicalMaterial material, inout ReflectedLight reflectedLight)
{
    float NoL = Saturate(dot(geometryNormal, directLight.Direction));
    vec3 irradiance = NoL * directLight.Color;

    float NoLcc = Saturate(dot(geometryClearcoatNormal, directLight.Direction));
    vec3 ccIrradiance = NoLcc * directLight.Color;

    reflectedLight.DirectDiffuse += irradiance * BRDF_Lambert(material.DiffuseColor);
    reflectedLight.DirectSpecular += irradiance * BRDF_GGX(directLight.Direction, geometryViewDir, geometryNormal, material);
}

void RE_IndirectDiffuse_Physical(const in vec3 irradiance, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in PhysicalMaterial material, inout ReflectedLight reflectedLight)
{
    reflectedLight.IndirectDiffuse += irradiance * BRDF_Lambert(material.DiffuseColor);
}

// Analytical approximation of the DFG LUT, one half of the
// split-sum approximation used in indirect specular lighting.
// via 'environmentBRDF' from "Physically Based Shading on Mobile"
// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
vec2 DFGApprox(const in vec3 normal, const in vec3 viewDir, const in float roughness)
{
    float dotNV = Saturate(dot(normal, viewDir));

    const vec4 c0 = vec4(- 1, - 0.0275, - 0.572, 0.022);
    const vec4 c1 = vec4(1, 0.0425, 1.04, - 0.04);

    vec4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(- 9.28 * dotNV)) * r.x + r.y;
    vec2 fab = vec2(- 1.04, 1.04) * a004 + r.zw;

    return fab;
}

void ComputeMultiscattering(const in vec3 normal, const in vec3 viewDir, const in vec3 specularColor, const in float specularF90, const in float roughness, inout vec3 singleScatter, inout vec3 multiScatter)
{
    vec2 fab = DFGApprox(normal, viewDir, roughness); // TODO: Make an option for switching to brdfLUT

    vec3 Fr = specularColor;
    vec3 FssEss = Fr * fab.x + specularF90 * fab.y;

    float Ess = fab.x + fab.y;
    float Ems = 1.0 - Ess;

    vec3 Favg = Fr + (1.0 - Fr) * 0.047619; // 1/21
    vec3 Fms = FssEss * Favg / (1.0 - Ems * Favg);

    singleScatter += FssEss;
    multiScatter += Fms * Ems;
}

void RE_IndirectSpecular_Physical(const in vec3 radiance, const in vec3 irradiance, const in vec3 clearcoatRadiance, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in PhysicalMaterial material, inout ReflectedLight reflectedLight)
{
    vec3 singleScattering = vec3(0.0);
    vec3 multiScattering = vec3(0.0);

    vec3 cosineWeightedIrradiance = irradiance * RECIPROCAL_PI;

    ComputeMultiscattering(geometryNormal, geometryViewDir, material.SpecularColor, material.SpecularF90, material.Roughness, singleScattering, multiScattering);

    vec3 totalScattering = singleScattering + multiScattering;
    vec3 diffuse = material.DiffuseColor * (1.0 - max(max(totalScattering.r, totalScattering.g), totalScattering.b));

    reflectedLight.IndirectSpecular += radiance * singleScattering;
    reflectedLight.IndirectSpecular += multiScattering * cosineWeightedIrradiance;
    reflectedLight.IndirectDiffuse += diffuse * cosineWeightedIrradiance;
}

//vec3 GetIBLIrradiance(const in vec3 normal)
//{
//    vec3 worldNormal = normal;
//    float lod = PerceptualRoughnessToLod(BRDFCtx, perceptualRoughness);
//    vec4 envMapColor = textureCubeUV(envMap, envMapRotation * worldNormal, 1.0);
//
//    return PI * envMapColor.rgb * envMapIntensity;
//}

vec3 GetIBLIrradiance(const in vec3 normal, const in float lod)
{
    return PI * textureLod(u_IrradianceMap, normal, lod).rgb;
}

vec3 GetIBLRadiance(const in vec3 normal, const in vec3 viewDir, const in float roughness)
{
    vec3 reflectVec = reflect(-viewDir, normal);
    // Mixing the reflection with the normal is more accurate and keeps rough objects from gathering light from behind their tangent plane.
    reflectVec = normalize(mix(reflectVec, normal, roughness * roughness));
//    DebugColor = reflectVec;
    reflectVec = InverseTransformDirection(reflectVec, u_ViewMatrix);

    float lod = 4.0 * roughness * (2.0 - roughness);

    return textureLod(u_PrefilterMap, reflectVec, lod).rgb;
}