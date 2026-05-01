#version 460

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_Direction;
layout(location = 1) out float v_Roughness;

layout(push_constant) uniform PushConstants {
    int u_FaceIndex;
    float u_Roughness;
} PC;

vec3 GetDirection(const in int face, const in vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;

     switch (face) {
        case 0: return vec3( 1.0, -ndc.y, -ndc.x); // +X (Right)
        case 1: return vec3(-1.0, -ndc.y,  ndc.x); // -X (Left)
        case 2: return vec3( ndc.x,  1.0,  ndc.y); // +Y (Top)
        case 3: return vec3( ndc.x, -1.0, -ndc.y); // -Y (Bottom)
        case 4: return vec3( ndc.x, -ndc.y,  1.0); // +Z (Back)
        case 5: return vec3(-ndc.x, -ndc.y, -1.0); // -Z (Front)
        default: return vec3(0.0);
    }
}

void main() {
    v_Direction = normalize(GetDirection(PC.u_FaceIndex, a_TexCoord));
    v_Roughness = PC.u_Roughness;

    gl_Position = vec4(a_Position * 2.0, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 v_Direction;
layout(location = 1) in float v_Roughness;

layout(location = 0) out vec4 o_Color;

layout(set = 1, binding = 0) uniform samplerCube u_Texture;

const uint NumSamples = 1024;
const float InvNumSamples = float(NumSamples);

#include "shaders://Includes/PBRAlgorithms.glsl"

void main()
{
	vec3 N = normalize(v_Direction);
    vec3 Lo = N;
    vec3 S, T;
	ComputeBasisVectors(N, S, T);

    // Solid angle associated with a single cubemap texel at zero mipmap level.
	// This will come in handy for importance sampling below.
    vec2 inputSize = vec2(textureSize(u_Texture, 0));
	float wt = 4.0 * PI / (6 * inputSize.x * inputSize.y);

	vec3 color = vec3(0);
	float weight = 0;
    
    // Convolve environment map using GGX NDF importance sampling.
	// Weight by cosine term since Epic claims it generally improves quality.
	for(uint i = 0; i < NumSamples; i++)
	{
		vec2 u = SampleHammersley(i, InvNumSamples);
		vec3 Lh = TangentToWorld(SampleGGX(u.x, u.y, v_Roughness), N, S, T);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		vec3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if(cosLi > 0.0) {
			// Use Mipmap Filtered Importance Sampling to improve convergence.
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

			float cosLh = max(dot(N, Lh), 0.0);

			// GGX normal distribution function (D term) probability density function.
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out).
			float pdf = NdfGGX(cosLh, v_Roughness) * 0.25;

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

    // vec3 testColor = texture(u_Texture, N).rgb;
    vec3 testColor = vec3(1.0, 1.0, 1.0);
    o_Color = vec4(testColor, 1.0);
}

#endif
