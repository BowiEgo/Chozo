#include "./Math.glsl"
#include "./GBuffer.glsl"

layout(binding = 4) uniform samplerCube u_IrradianceMap;
layout(binding = 5) uniform samplerCube u_PrefilterMap;
layout(binding = 6) uniform sampler2D u_BRDFLutTex;

struct BRDFContext
{
    highp float NoV;
    vec3 f0;
    vec3 dfg;

    vec3 EnergyCompensation;
    float IBLRoughnessOneLevel;
} BRDFCtx;

vec3 SpecularDFG(const BRDFContext BRDFCtx)
{
    return mix(BRDFCtx.dfg.yyy, BRDFCtx.dfg.xxx, BRDFCtx.f0);
//    return BRDFCtx.f0 * BRDFCtx.dfg.x + BRDFCtx.dfg.y;
}

float PerceptualRoughnessToLod(const BRDFContext BRDFCtx, float perceptualRoughness)
{
    return BRDFCtx.IBLRoughnessOneLevel * perceptualRoughness * (2.0 - perceptualRoughness);
}

vec3 PrefilteredRadiance(const BRDFContext BRDFCtx, const vec3 r, float perceptualRoughness)
{
    float lod = PerceptualRoughnessToLod(BRDFCtx, perceptualRoughness);
    return textureLod(u_PrefilterMap, r, lod).rgb;
}

vec3 DiffuseIrradiance(const vec3 n, const BRDFContext BRDFCtx)
{
    return textureLod(u_IrradianceMap, n, BRDFCtx.IBLRoughnessOneLevel).rgb;
}

void InitBRDFContext(const GBufferData GBuffer, out BRDFContext BRDFCtx)
{
    BRDFCtx.NoV = Saturate(max(min(dot(GBuffer.Normal, GBuffer.View), 0.999), 0.01));
    BRDFCtx.f0 = 0.16 * GBuffer.Reflectance * GBuffer.Reflectance * (1.0 - GBuffer.Metallic)
        + GBuffer.BaseColor * GBuffer.Metallic;
    BRDFCtx.dfg = textureLod(u_BRDFLutTex, vec2(BRDFCtx.NoV, GBuffer.PerceptualRoughness), 0.0).rgb;

    BRDFCtx.EnergyCompensation = vec3(1.0);
    BRDFCtx.IBLRoughnessOneLevel = 4.0;
}

//------------------------------------------------------------------------------
// Specular BRDF implementations
//------------------------------------------------------------------------------
float D_GGX(float roughness, float NoH, const vec3 h)
{
    // Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"

    // In mediump, there are two problems computing 1.0 - NoH^2
    // 1) 1.0 - NoH^2 suffers floating point cancellation when NoH^2 is close to 1 (highlights)
    // 2) NoH doesn't have enough precision around 1.0
    // Both problem can be fixed by computing 1-NoH^2 in highp and providing NoH in highp as well

    // However, we can do better using Lagrange's identity:
    //      ||a x b||^2 = ||a||^2 ||b||^2 - (a . b)^2
    // since N and H are unit vectors: ||N x H||^2 = 1.0 - NoH^2
    // This computes 1.0 - NoH^2 directly (which is close to zero in the highlights and has
    // enough precision).
    // Overall this yields better performance, keeping all computations in mediump
    float oneMinusNoHSquared = 1.0 - NoH * NoH;

    float a = NoH * roughness;
    float k = roughness / (oneMinusNoHSquared + a * a);
    float d = k * k * (1.0 / PI);
    return saturateMediump(d);
}

vec3 F_Schlick(const vec3 f0, float f90, float VoH)
{
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

vec3 F_Schlick(const vec3 f0, float VoH)
{
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

float F_Schlick(float f0, float f90, float VoH)
{
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

float V_SmithGGXCorrelated(float roughness, float NoV, float NoL)
{
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = roughness * roughness;
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float lambdaL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    float v = 0.5 / (lambdaV + lambdaL);
    // a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
    // a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
    // clamp to the maximum value representable in mediump
    return saturateMediump(v);
}

//------------------------------------------------------------------------------
// Diffuse BRDF implementations
//------------------------------------------------------------------------------
float Fd_Lambert()
{
    return 1.0 / PI;
}

float Fd_Burley(float roughness, float NoV, float NoL, float LoH)
{
    // Burley 2012, "Physically-Based Shading at Disney"
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_Schlick(1.0, f90, NoL);
    float viewScatter  = F_Schlick(1.0, f90, NoV);
    return lightScatter * viewScatter * (1.0 / PI);
}
