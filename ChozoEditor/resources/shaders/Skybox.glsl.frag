#version 450

layout (location = 0) in vec3 v_TexCoords;
layout(location = 0) out vec4 o_Color;

layout(binding = 0) uniform samplerCube u_Texture;

layout (push_constant) uniform Uniforms
{
	float TextureLod;
	float Intensity;
} u_FragUniforms;

void main()
{
	o_Color = textureLod(u_Texture, v_TexCoords, u_FragUniforms.TextureLod) * u_FragUniforms.Intensity;
	o_Color.a = 1.0;
}