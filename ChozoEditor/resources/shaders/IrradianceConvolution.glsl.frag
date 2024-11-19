#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_WorldPosition;

layout(binding = 0) uniform samplerCube u_Texture;

// layout(push_constant) uniform Uniforms
// {
// 	uint Samples;
// } u_FragUniforms;

#include "includes/PBRAlgorithms.glsl"

void main()
{
	// The world vector acts as the normal of a tangent surface
    // from the origin, aligned to v_WorldPosition. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    vec3 N = normalize(v_WorldPosition);
    vec3 S, T;
	ComputeBasisVectors(N, S, T);

    vec3 irradiance = vec3(0.0);
	// uint samples = 64 * u_FragUniforms.Samples;
    uint samples = 64 * 512;

    for(uint i = 0; i < samples; i++)
	{
		vec2 u  = SampleHammersley(i, float(samples));
		vec3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
		float cosTheta = max(0.0, dot(Li, N));

		// PIs here cancel out because of division by pdf.
		irradiance += 2.0 * textureLod(u_Texture, Li, 0).rgb * cosTheta;
	}
	irradiance /= vec3(samples);

    o_Color = vec4(irradiance, 1.0);
}