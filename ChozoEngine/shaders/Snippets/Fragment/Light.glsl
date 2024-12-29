struct DirectionalLight {
    highp vec3 Direction;
    float Intensity;
    vec3 Color;
};

struct PointLight
{
    highp vec3 Position;
    float Intensity;
    vec3 Color;
};

struct SpotLight
{
    highp vec3 Position;
    float Intensity;
    highp vec3 Direction;
    float AngleAttenuation;
    vec3 Color;
    float Angle;
};

struct Light
{
    vec3 l;
    vec3 h;
    vec3 Color;
    float Intensity;
    float Attenuation;
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

