#include "../Snippets/Fragment/Light.glsl"
#include "./Math.glsl"
#include "./GBuffer.glsl"
#include "./BRDF.glsl"

//------------------------------------------------------------------------------
// Indirect Light
//------------------------------------------------------------------------------
vec3 GetSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

vec3 GetReflectedVector(const vec3 r, const vec3 n, const float roughness) {
    return GetSpecularDominantDirection(n, r, roughness);
}

vec3 GetReflectedVector(const GBufferData GBuffer, BRDFContext BRDFCtx) {
    return 2.0 * dot(GBuffer.View, GBuffer.Normal) * GBuffer.Normal - GBuffer.View;
}

vec3 EvaluateIBL(const GBufferData GBuffer, BRDFContext BRDFCtx)
{
    vec3 color = vec3(0.0);

    // specular layer
    vec3 Fr = vec3(0.0);
    vec3 E = SpecularDFG(BRDFCtx);
//    vec3 r = GetReflectedVector(GBuffer.Reflected, GBuffer.Normal, GBuffer.Roughness);
    vec3 r = GetReflectedVector(GBuffer, BRDFCtx);
    Fr = E * PrefilteredRadiance(BRDFCtx, r, GBuffer.PerceptualRoughness);

    // diffuse layer
    vec3 Fd = vec3(0.0);
    vec3 diffuseIrradiance = DiffuseIrradiance(GBuffer.Normal, BRDFCtx);
    float diffuseBRDF = GBuffer.AO;
    Fd = GBuffer.BaseColor * diffuseIrradiance * (1.0 - E) * diffuseBRDF;

    // subsurface layer(TODO)

    // sheen layer(TODO)

    // clear coat layer(TODO)

    color += Fr + Fd;

    return color;
}

//------------------------------------------------------------------------------
// Directional Light & Punctual Light
//------------------------------------------------------------------------------
float ComputeMicroShadowing(float NoL, float visibility)
{
    // Chan 2018, "Material Advances in Call of Duty: WWII"
    float aperture = inversesqrt(1.0 - min(visibility, 0.9999));
    float microShadow = Saturate(NoL * aperture);
    return microShadow * microShadow;
}

float GetDistanceAttenuation(const highp vec3 posToLight, float intensity, float range)
{
    float distance = length(posToLight);
    if (distance == 0.0) {
        return intensity;
    }

    float attenuation = intensity / (distance * distance);

    if (distance > range) {
        attenuation = 0.0;
    }

    return attenuation;
}

float GetAngleAttenuation(vec3 lightDir, vec3 spotDir, float cosOuter, float cosInner) {
    float cosTheta = dot(lightDir, spotDir);
    float smoothFactor = smoothstep(cosOuter, cosInner, cosTheta);
    return smoothFactor;
}

vec3 SurfaceShading(const GBufferData GBuffer, BRDFContext BRDFCtx, Light light)
{
    float NoV = BRDFCtx.NoV;
    float NoL = Saturate(dot(GBuffer.Normal, light.l));
    float NoH = Saturate(dot(GBuffer.Normal, light.h));
    float LoH = Saturate(dot(light.l, light.h));

    float D = D_GGX(GBuffer.Roughness, NoH, light.h);
    float V = V_SmithGGXCorrelated(GBuffer.Roughness, NoV, NoL);
    vec3  F = F_Schlick(BRDFCtx.f0, LoH); // f90 = 1.0

    vec3 Fr = (D * V) * F;
    vec3 Fd = GBuffer.BaseColor * Fd_Burley(GBuffer.Roughness, NoV, NoL, LoH);

    vec3 color = Fd + Fr * BRDFCtx.EnergyCompensation;
    color *= light.Attenuation;

    float visibility = 1.0;
    visibility *= ComputeMicroShadowing(NoL, GBuffer.AO);

    float occlusion = visibility;

    float illuminance = light.Intensity * NoL;
    return (color * light.Color) *
    (illuminance * 1.0 * NoL * occlusion);
}

vec3 EvaluateDirectionalLight(const GBufferData GBuffer, const BRDFContext BRDFCtx, const DirectionalLight dirLight)
{
    highp vec3 posToLight = dirLight.Direction;

    Light light;
    light.l = normalize(posToLight);
    light.h = normalize(GBuffer.View + light.l);
    light.Intensity = dirLight.Intensity;
    light.Color = dirLight.Color;
    light.Attenuation = 1.0;
    return SurfaceShading(GBuffer, BRDFCtx, light);
}

vec3 EvaluatePunctualLight(const GBufferData GBuffer, const BRDFContext BRDFCtx, const PointLight pointLight)
{
    highp vec3 posToLight = pointLight.Position - GBuffer.Position;

    Light light;
    light.l = normalize(posToLight);
    light.h = normalize(GBuffer.View + light.l);
    light.Intensity = pointLight.Intensity;
    light.Color = pointLight.Color;
    light.Attenuation = GetDistanceAttenuation(posToLight, pointLight.Intensity, 10.0);
    return SurfaceShading(GBuffer, BRDFCtx, light);
}


/**
 * This function evaluates all lights one by one:
 * - Image based lights (IBL)
 * - Directional lights
 * - Punctual lights
 *
 * Area lights are currently not supported.
 *d
 * Returns a pre-exposed HDR RGBA color in linear space.
 */
vec3 EvaluateLights(const GBufferData GBuffer, BRDFContext BRDFContext)
{
    vec3 color = vec3(0.0);

    color += EvaluateIBL(GBuffer, BRDFContext);

    for (int i = 0; i < u_DirectionalLights.LightCount; i++)
    {
        DirectionalLight dirLight = u_DirectionalLights.Lights[i];
        color += EvaluateDirectionalLight(GBuffer, BRDFContext, dirLight);
    }

    for (int i = 0; i < u_PointLights.LightCount; i++)
    {
        PointLight pointLight = u_PointLights.Lights[i];
        color += EvaluatePunctualLight(GBuffer, BRDFContext, pointLight);
    }
    return color;
}
