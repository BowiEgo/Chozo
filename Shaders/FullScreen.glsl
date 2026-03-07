#version 450

// [Note] Added 'uniform' keyword to fix ShaderC error
#ifdef FRAGMENT_SHADER
layout(binding = 0) uniform sampler2D u_SceneTexture;
#endif

#ifdef VERTEX_SHADER
layout(location = 0) out vec2 outUV;

void main() {
    // [Note] Fullscreen triangle generation logic
    float x = -1.0 + float((gl_VertexIndex & 1) << 2);
    float y = -1.0 + float((gl_VertexIndex & 2) << 1);
    
    outUV.x = (x + 1.0) * 0.5;
    outUV.y = (y + 1.0) * 0.5;

    // [Note] Standard Vulkan NDC Y-flip
    gl_Position = vec4(x, -y, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main() {
    // [Note] Direct sample from the scene texture
    outColor = texture(u_SceneTexture, inUV);
}
#endif