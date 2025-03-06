#version 450

layout(location = 0) out vec4 o_Color;

layout (location = 0) in vec3 v_TexCoords;

layout(binding = 0) uniform samplerCube u_Texture;

layout (push_constant) uniform PushConstants
{
	float TextureLod;
	float Intensity;
} u_Constant;

void main()
{
	vec4 texture = textureLod(u_Texture, v_TexCoords, u_Constant.TextureLod) * u_Constant.Intensity;
	vec3 color = texture.rgb;

	color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

	o_Color = vec4(color, texture.a);
}