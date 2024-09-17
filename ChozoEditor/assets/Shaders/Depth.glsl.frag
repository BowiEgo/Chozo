#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;

float near = 0.1; 
float far  = 100.0; 

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
    float depth = LinearizeDepth(v_FragPosition.z) / far;

    o_Color = vec4(vec3(depth), 1.0);
}
