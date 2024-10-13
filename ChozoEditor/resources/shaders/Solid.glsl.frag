#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Depth;
layout(location = 2) out int o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;
layout(location = 3) in flat int v_EntityID;

struct DirectionalLight
{
	vec3 Direction;
	float Intensity;
    vec3 Color;
};

layout(std140, binding = 0) uniform CameraData
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    mat4 u_InverseViewProjectionMatrix;
};

layout(std140, binding = 1) uniform SceneData
{
	DirectionalLight DirectionalLights;
	vec3 CameraPosition; // Offset = 32
	float EnvironmentMapIntensity;
} u_Scene;

float near = 0.1;
float far  = 20.0;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
    float depth = LinearizeDepth(pow(gl_FragCoord.z, 0.3)) / far;

    vec3 viewDir = normalize(-vec3(u_ViewMatrix[0][2], u_ViewMatrix[1][2], u_ViewMatrix[2][2]));
    vec3 right = normalize(vec3(u_ViewMatrix[0][0], u_ViewMatrix[1][0], u_ViewMatrix[2][0]));
    vec3 up = normalize(vec3(u_ViewMatrix[0][1], u_ViewMatrix[1][1], u_ViewMatrix[2][1]));

    vec3 topLeftDirection = normalize(-viewDir + up * 0.5 - right * 0.5);

    vec3 normal = normalize(v_Normal);
    vec3 baseColor = vec3(1.0);

    float brightness = clamp(dot(normal, topLeftDirection), 0.3, 1.0);

    o_Color = vec4(baseColor * brightness, 1.0);
    o_Depth = vec4(vec3(depth), 1.0);
    o_EntityID = v_EntityID;
}