#version 460

// ===== Uniform =====
layout(set = 0, binding = 0) uniform CameraData {
    mat4 View;
    mat4 Projection;
} u_Camera;

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(push_constant) uniform VertexContant
{
    mat4 ModelMatrix;
    mat3 NormalMatrix;
} u_VertContant;

void main() {
    mat4 model = u_VertContant.ModelMatrix;
    mat4 mvp = u_Camera.Projection * u_Camera.View * model;
    
    gl_Position = mvp * vec4(a_Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 o_Color;

void main() {
    vec3 finalColor = vec3(0.5, 0.5, 0.0);
    
    o_Color = vec4(finalColor, 1.0);
}

#endif
