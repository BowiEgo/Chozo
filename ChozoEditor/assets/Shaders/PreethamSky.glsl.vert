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
    vec4 modelPosition = mat4(1.0) * vec4(a_Position, 1.0);
    gl_Position = camera.ProjectionMatrix * camera.ViewMatrix * modelPosition;
    v_WorldPosition = a_Position;
}