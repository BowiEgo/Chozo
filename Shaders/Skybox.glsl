#version 460

#ifdef VERTEX_SHADER

// ===== Uniform =====
#include "shaders://Camera.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_Direction;

void main() {
    vec4 pos = u_Camera.ProjMatrix * mat4(mat3(u_Camera.ViewMatrix)) * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
    v_Direction = a_Position;
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 v_Direction;

layout(location = 0) out vec4 o_Color;

layout(set = 1, binding = 0) uniform samplerCube u_Texture;

void main() {
    vec4 texture = textureLod(u_Texture, v_Direction, 1.0) * 1.0;
	vec3 color = texture.rgb;

	o_Color = vec4(color, texture.a);
}

#endif
