#include "GBuffer.glsl"

struct PhysicalMaterial
{
    vec3 DiffuseColor;
    float Roughness;
    vec3 SpecularColor;
    float SpecularF90;
    float Dispersion;

    float Clearcoat;
    float ClearcoatRoughness;
    vec3 ClearcoatF0;
    float ClearcoatF90;

    float Iridescence;
    float IridescenceIOR;
    float IridescenceThickness;
    vec3 IridescenceFresnel;
    vec3 IridescenceF0;

    vec3 SheenColor;
    float SheenRoughness;

    float IOR;

    float Transmission;
    float TransmissionAlpha;
    float Thickness;
    float AttenuationDistance;
    vec3 AttenuationColor;

    float Anisotropy;
    float AlphaT;
    vec3 AnisotropyT;
    vec3 AnisotropyB;
};

struct MaterialInputs {
    vec4 BaseColor;
    float Roughness;
    float Metallic;
    float Reflectance;
    float AmbientOcclusion;
    vec4 Emissive;

    #if defined(MATERIAL_HAS_NORMAL)
    vec3 normal;
    #endif
};

void InitMaterial(out MaterialInputs material) {
    material.BaseColor = vec4(1.0);
    material.Roughness = 1.0;
    material.Metallic = 0.0;
    material.Reflectance = 0.5;
    material.AmbientOcclusion = 1.0;
    material.Emissive = vec4(vec3(1.0), 1.0);
    #if defined(MATERIAL_HAS_NORMAL)
    material.normal = vec3(0.0, 0.0, 1.0);
    #endif
}

vec4 EvaluateMaterial(const MaterialInputs material) {
    //    PixelParams pixel;
    //    GetPixelParams(material, pixel);

    //    vec4 color = EvaluateLights(material, pixel);
    //    AddEmissive(material, color);
    //    return color;
    return vec4(1.0);
}

void InitPhysicalMaterial(const in GBufferData GBuffer, out PhysicalMaterial material)
{
    vec3 nonPerturbedNormal = GBuffer.PerturbedNormal; // TODO: this is temporary
    vec3 dxy = max(abs(dFdx(nonPerturbedNormal)), abs(dFdy(nonPerturbedNormal)));
    float geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);

    material.DiffuseColor = GBuffer.BaseColor * (1.0 - GBuffer.Metallic);
    material.Roughness = max(GBuffer.Roughness, 0.0525);
    material.Roughness += geometryRoughness;

    material.SpecularColor = mix(vec3(0.04), GBuffer.BaseColor.rgb, GBuffer.Metallic);
    material.SpecularF90 = 1.0;
}