#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_WorldPosition;

layout(binding = 0) uniform samplerCube u_Texture;

layout(push_constant) uniform Uniforms
{
    float Roughness;
} u_FragUniforms;

const uint NumSamples = 1024;
const float InvNumSamples = float(NumSamples);

#include "Includes/PBRAlgorithms.glsl"

void main()
{
    // Solid angle associated with a single cubemap texel at zero mipmap level.
	// This will come in handy for importance sampling below.
    vec2 inputSize = vec2(textureSize(u_Texture, 0));
	float wt = 4.0 * PI / (6 * inputSize.x * inputSize.y);

	vec3 N = normalize(v_WorldPosition);

    vec3 Lo = N;
	
	vec3 S, T;
	ComputeBasisVectors(N, S, T);

	vec3 color = vec3(0);
	float weight = 0;
    
    // Convolve environment map using GGX NDF importance sampling.
	// Weight by cosine term since Epic claims it generally improves quality.
	for(uint i = 0; i < NumSamples; i++)
	{
		vec2 u = SampleHammersley(i, InvNumSamples);
		vec3 Lh = TangentToWorld(SampleGGX(u.x, u.y, u_FragUniforms.Roughness), N, S, T);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		vec3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if(cosLi > 0.0) {
			// Use Mipmap Filtered Importance Sampling to improve convergence.
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

			float cosLh = max(dot(N, Lh), 0.0);

			// GGX normal distribution function (D term) probability density function.
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out).
			float pdf = NdfGGX(cosLh, u_FragUniforms.Roughness) * 0.25;

			// Solid angle associated with this sample.
			float ws = 1.0 / (NumSamples * pdf);

			// Mip level to sample from.
			float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

			color  += textureLod(u_Texture, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
	}
	color /= weight;

    o_Color = vec4(color, 1.0);
}