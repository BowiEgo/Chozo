layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform CameraData
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    mat4 u_InverseViewProjectionMatrix;
};

layout(push_constant) uniform VertexUniforms
{
    mat4 ModelMatrix;
    mat3 NormalMatrix;
} u_VertUniforms;

layout(location = 0) out vec3 v_WorldNormal;
layout(location = 1) out vec3 v_WorldTangent;
layout(location = 2) out vec3 v_WorldBitangent;
layout(location = 3) out vec2 v_TexCoord;
layout(location = 4) out vec3 v_FragPosition;
