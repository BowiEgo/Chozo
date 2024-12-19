layout(location = 0) out vec4 o_Color;

#include "Common/SceneData.glsl"
#include "Common/Lighting.glsl"

layout(binding = 5) uniform samplerCube s_IBLSpecularMap;

highp vec3  shading_position;         // position of the fragment in world space
      vec3  shading_view;             // normalized vector from the fragment to the eye
      vec3  shading_normal;           // normalized transformed normal, in world space
      vec3  shading_reflected;        // reflection of view about normal

#define MIN_PERCEPTUAL_ROUGHNESS 0.045
#define MIN_ROUGHNESS            0.002025

#define MAX_REFLECTION_LOD 4.0

struct PixelParams {
    vec3 diffuseColor;
    float perceptualRoughness;
    float roughness;
    vec3  dfg;
};

struct MaterialInputs {
    vec4 baseColor;
    float roughness;
    float metallic;
    float reflectance;
    float ambientOcclusion;
    vec4 emissive;

#if defined(MATERIAL_HAS_NORMAL)
    vec3  normal;
#endif
};

float Saturate(float x) { return clamp(x, 0.0, 1.0); }

void InitMaterial(out MaterialInputs material) {
    material.baseColor = vec4(1.0);
    material.roughness = 1.0;
    material.metallic = 0.0;
    material.reflectance = 0.5;
    material.ambientOcclusion = 1.0;
    material.emissive = vec4(vec3(1.0), 1.0);
#if defined(MATERIAL_HAS_NORMAL)
    material.normal = vec3(0.0, 0.0, 1.0);
#endif
}

vec3 GetSpecularDFG(const PixelParams pixel) {
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0);
}

vec3 GetSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

vec3 GetReflectedVector(const PixelParams pixel, const vec3 n) {
#if defined(MATERIAL_HAS_ANISOTROPY)
#else
    vec3 r = shading_reflected;
#endif
    return GetSpecularDominantDirection(n, r, pixel.roughness);
}

float GerceptualRoughnessToLod(float perceptualRoughness) {
    return MAX_REFLECTION_LOD * perceptualRoughness * (2.0 - perceptualRoughness);
}

vec3 PrefilteredRadiance(const vec3 R, float perceptualRoughness) {
    float Lod = GerceptualRoughnessToLod(perceptualRoughness);
    return textureLod(s_IBLSpecularMap, R, Lod).rgb;
}

vec3 EvaluateIBL(const PixelParams pixel) {
    // Specular layer
    vec3 Fr = vec3(0.0);

#if IBL_INTEGRATION == IBL_INTEGRATION_PREFILTERED_CUBEMAP
    vec3 E = GetSpecularDFG(pixel);
    vec3 R = GetReflectedVector(pixel, shading_normal);
    Fr = E * PrefilteredRadiance(R, pixel.perceptualRoughness);
#elif IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING

#endif

    // Ambient occlusion

    // Diffuse layer
    vec3 Fd = vec3(0.0);

#if IBL_INTEGRATION == IBL_INTEGRATION_PREFILTERED_CUBEMAP
    vec3 diffuseIrradiance = diffuseIrradiance(diffuseNormal);
#elif IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING
#endif
    Fd = pixel.diffuseColor * diffuseIrradiance * (1.0 - E);

    return Fr + Fd;
}

vec3 EvaluateDirectionalLight(const PixelParams pixel) {
    vec3 h = normalize(shading_view + light.l);

    float NoV = shading_NoV;
    float NoL = Saturate(light.NoL);
    float NoH = Saturate(dot(shading_normal, h));
    float LoH = Saturate(dot(light.l, h));

    float D = D_GGX(NoH, pixel.roughness);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, pixel.roughness);

    vec3 Fr = (D * V) * F;
    vec3 Fd = pixel.diffuseColor * Fd_Lambert();

    vec3 color = Fd + Fr * pixel.energyCompensation;

    DirectionalLight light = u_Scene.directionalLight;
    float occlusion = 1.0; // visibility
    return (color * light.color) *
        (light.intensity * 1.0 * NoL * occlusion);
}

vec4 EvaluateLights(const MaterialInputs material, const PixelParams pixel) {
    vec3 color = vec3(0.0);

//    color += EvaluateIBL(pixel);

    color += EvaluateDirectionalLight(pixel);

//    color += EvaluatePunctualLights(pixel);

    color *= material.baseColor.a;

    return vec4(color, 1.0);
}

void GetPixelParams(const MaterialInputs material, out PixelParams pixel) {
    float perceptualRoughness = material.roughness;

    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    pixel.perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.roughness = pixel.perceptualRoughness * pixel.perceptualRoughness;
}

void EvaluateMaterial(const MaterialInputs material) {
    PixelParams pixel;
    GetPixelParams(material, pixel);

    vec4 color = EvaluateLights(material, pixel);
//    AddEmissive(material, color);
    return color;
}

void main() {
    MaterialInputs inputs;
    InitMaterial(inputs);

//    // Invoke user custom code
//    material(inputs);

    o_Color = EvaluateMaterial(inputs);
}