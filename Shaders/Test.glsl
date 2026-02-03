#version 450

// English Comment: Common structures or constants can be placed here

#ifdef VERTEX_SHADER
// English Comment: Attributes from Vertex Buffer
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Color;

// English Comment: Output to Fragment Shader
layout(location = 0) out vec3 v_Color;

void main() {
    v_Color = a_Color;
    
    // English Comment: Standard gl_Position output
    gl_Position = vec4(a_Pos, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
// English Comment: Input from Vertex Shader (interpolated)
layout(location = 0) in vec3 v_Color;

// English Comment: Final output color to Framebuffer
layout(location = 0) out vec4 o_Color;

void main() {
    // English Comment: Output the interpolated color with full opacity
    o_Color = vec4(v_Color, 1.0);
}
#endif