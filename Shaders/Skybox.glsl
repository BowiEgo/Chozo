#version 460

#ifdef VERTEX_SHADER

// ===== Uniform =====
#include "shaders://Camera.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_TexCoords;

void main() {
    vec4 pos = u_Camera.Projection * mat4(mat3(u_Camera.View)) * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
    v_TexCoords = a_Position;
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 o_Color;

layout (location = 0) in vec3 v_TexCoords;

layout(set = 0, binding = 1) uniform samplerCube u_Texture;

void main() {
    vec4 texture = textureLod(u_Texture, v_TexCoords, 1.0) * 1.0;
	vec3 color = texture.rgb;

	// color = color / (color + vec3(1.0));
	// color = pow(color, vec3(1.0/2.2));

	o_Color = vec4(color, texture.a);
}

#endif
