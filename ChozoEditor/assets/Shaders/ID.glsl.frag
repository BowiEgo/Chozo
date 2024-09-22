#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;

layout(binding = 0) uniform isampler2D u_IdText;

vec3 getRandomColor(int id) {
    float r = float((id * 37) % 255) / 255.0;
    float g = float((id * 57) % 255) / 255.0;
    float b = float((id * 73) % 255) / 255.0;
    return vec3(r, g, b);
}

void main()
{
    int value = texture(u_IdText, v_TexCoord).r;

    o_Color = vec4(getRandomColor(value), 1.0);
}