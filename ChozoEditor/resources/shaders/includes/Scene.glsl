struct DirectionalLight
{
	vec3 Direction;
	float Intensity;
    vec3 Color;
};

layout(std140, binding = 1) uniform SceneData
{
	DirectionalLight DirectionalLights;
	vec3 CameraPosition; // Offset = 32
	float EnvironmentMapIntensity;
} u_Scene;

struct PointLight
{
	vec3 Position;
	float Intensity;
	vec3 Color;
};

layout(std140, binding = 2) uniform PointLightData
{
	uint LightCount;
	PointLight Lights[1000];
} u_PointLights;

struct SpotLight
{
    vec3 Position;
    float Intensity;
    vec3 Direction;
    float AngleAttenuation;
    vec3 Color;
    float Angle;
};

layout(std140, binding = 3) uniform SpotLightData
{
	uint LightCount;
	SpotLight Lights[1000];
} u_SpotLights;