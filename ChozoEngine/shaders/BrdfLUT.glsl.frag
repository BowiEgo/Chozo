#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

#include "Includes/Math.glsl"
#include "Includes/PBRAlgorithms.glsl"

// ----------------------------------------------------------------------------
vec2 IntegrateBRDF(float NoV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NoV*NoV);
    V.y = 0.0;
    V.z = NoV;

    float A = 0.0;
    float B = 0.0; 

    vec3 N = vec3(0.0, 0.0, 1.0);
    vec3 S, T;
	ComputeBasisVectors(N, S, T);
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
		vec2 u = SampleHammersley(i, float(SAMPLE_COUNT));
		vec3 H = TangentToWorld(SampleGGX(u.x, u.y, roughness), N, S, T);

        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NoL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NoL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NoV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}
// ----------------------------------------------------------------------------
void main() 
{
    vec2 integratedBRDF = IntegrateBRDF(v_TexCoord.x, v_TexCoord.y);
    o_Color = vec4(integratedBRDF, 0.0, 1.0);
}