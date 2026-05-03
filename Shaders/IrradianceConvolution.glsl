#version 460

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_Direction;

layout(push_constant) uniform PushConstants {
    int u_FaceIndex;
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
    gl_Position = vec4(a_Position * 2.0, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 v_Direction;

layout(location = 0) out vec4 o_Color;

layout(set = 1, binding = 0) uniform samplerCube u_Texture;

#include "shaders://Includes/PBRAlgorithms.glsl"

void main()
{
	// The world vector acts as the normal of a tangent surface
    // from the origin, aligned to v_Direction. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    vec3 N = normalize(v_Direction);
    vec3 S, T;
	ComputeBasisVectors(N, S, T);

    vec3 irradiance = vec3(0.0);
	// uint samples = 64 * u_FragUniforms.Samples;
    uint samples = 32 * 512;
    // uint samples = 512;

    for(uint i = 0; i < samples; i++)
	{
		vec2 u  = SampleHammersley(i, float(samples));
		vec3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
		float cosTheta = max(0.0, dot(Li, N));

		// PIs here cancel out because of division by pdf.
		irradiance += textureLod(u_Texture, Li, 0).rgb * cosTheta;
	}
	// irradiance /= vec3(samples);
    // o_Color = vec4(irradiance, 1.0);
    o_Color = vec4(PI * irradiance / float(samples), 1.0);

    // vec4 texture = textureLod(u_Texture, v_Direction, 1.0) * 1.0;
	// vec3 color = texture.rgb;

    // o_Color = vec4(v_Direction, 1.0);
	// o_Color = vec4(color, texture.a);
}

#endif
