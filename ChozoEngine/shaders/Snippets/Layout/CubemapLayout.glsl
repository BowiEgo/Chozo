layout (location = 0) in vec3 a_Position;

layout (push_constant) uniform View
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} u_View;

layout(location = 0) out vec3 v_WorldPosition;