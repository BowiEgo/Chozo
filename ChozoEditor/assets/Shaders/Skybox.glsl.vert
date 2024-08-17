#version 450

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    mat4 u_InverseViewProjectionMatrix;
};

layout(location = 0) out vec3 v_TexCoords;

void main()
{
    v_TexCoords = a_Position;
    // vec4 position = vec4(a_Position.xy, 0.0, 1.0);
	// v_TexCoords = (u_InverseViewProjectionMatrix * position).xyz;

    vec4 pos = u_ProjectionMatrix * u_ViewMatrix * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;

	// gl_Position = position;
    // gl_Position = pos;
}