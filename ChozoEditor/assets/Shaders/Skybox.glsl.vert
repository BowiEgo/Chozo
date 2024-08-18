#version 450

layout(location = 0) in vec3 a_Position;

layout (push_constant) uniform Camera
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewProjectionMatrix;
} camera;

layout(location = 0) out vec3 v_TexCoords;

void main()
{
    v_TexCoords = a_Position;
    vec4 pos = camera.ProjectionMatrix * camera.ViewMatrix * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;

    // vec4 position = vec4(a_Position.xy, 0.0, 1.0);
	// v_TexCoords = (camera.InverseViewProjectionMatrix * position).xyz;
	// gl_Position = position;
}