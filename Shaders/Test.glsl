#version 450

// ===== Uniform =====
layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
} camera;

#ifdef VERTEX_SHADER

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 v_Color;

void main() {
    mat4 model = mat4(1.0);
    
    mat4 mvp = camera.projection * camera.view * model;
    
    gl_Position = mvp * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    
    v_Color = colors[gl_VertexIndex];
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 v_Color;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(v_Color, 1.0);
}

#endif