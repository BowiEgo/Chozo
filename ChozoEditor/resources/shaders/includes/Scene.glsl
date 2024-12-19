layout(std140, binding = 1) uniform SceneData
{
	vec3 CameraPosition; // Offset = 32
	float EnvironmentMapIntensity;
} u_Scene;

struct DirectionalLight {
	vec3 Direction;
	float Intensity;
	vec3 Color;
};

struct PointLight
{
	vec3 Position;
	float Intensity;
	vec3 Color;
};

struct SpotLight
{
	vec3 Position;
	float Intensity;
	vec3 Direction;
	float AngleAttenuation;
	vec3 Color;
	float Angle;
};

layout(std140, binding = 2) uniform DirectionalLightsData
{
	uint LightCount;
	DirectionalLight Lights[1000];
} u_DirectionalLights;

layout(std140, binding = 3) uniform PointLightsData
{
	uint LightCount;
	PointLight Lights[1000];
} u_PointLights;

layout(std140, binding = 4) uniform SpotLightsData
{
	uint LightCount;
	SpotLight Lights[1000];
} u_SpotLights;

