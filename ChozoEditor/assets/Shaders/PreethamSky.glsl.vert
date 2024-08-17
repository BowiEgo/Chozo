#version 450

layout (location = 0) in vec3 a_Position;

layout (push_constant) uniform Camera
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} camera;

layout(location = 0) out vec3 v_WorldPosition;

void main()
{
    v_WorldPosition = a_Position;
    gl_Position = camera.ProjectionMatrix * camera.ViewMatrix * vec4(a_Position, 1.0);
}