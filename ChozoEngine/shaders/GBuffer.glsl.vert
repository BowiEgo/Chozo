#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec3 a_Tangent;

layout(std140, binding = 0) uniform CameraData
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    mat4 u_InverseViewProjectionMatrix;
};

layout(push_constant) uniform VertexUniforms
{
    mat4 ModelMatrix;
} u_VertUniforms;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out vec3 v_FragPosition;

void main()
{
    vec4 modelPosition = u_VertUniforms.ModelMatrix * vec4(a_Position, 1.0);
    vec4 viewPosition = u_ViewMatrix * modelPosition;
    vec4 projectionPosition = u_ProjectionMatrix * viewPosition;

    gl_Position = projectionPosition;

    mat3 normalMatrix = transpose(inverse(mat3(u_VertUniforms.ModelMatrix)));
    v_Normal = normalMatrix * a_Normal;
    v_TexCoord = a_TexCoord;
    v_FragPosition = vec3(modelPosition);
}