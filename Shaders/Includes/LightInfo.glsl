/**
 * -- Indirect Light
 *    |- Image Based Light (IBL)
 * -- Direct Light
 *    |- Image Based Light (IBL) (currently not supported)
 *    |- Directional Lights
 *    |- Punctual Lights
 *       |- Point Lights
 *       |- Spot Lights
 *       |- Area Lights (currently not supported)
 */

struct IncidentLight {
    vec3 Color;
    vec3 Direction;
    bool Visible;
};

struct ReflectedLight {
    vec3 DirectDiffuse;
    vec3 DirectSpecular;
    vec3 IndirectDiffuse;
    vec3 IndirectSpecular;
};

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

struct Light
{
    vec3 l;
    vec3 h;
    vec3 Color;
    float Intensity;
    float Attenuation;
};

// layout(std140, binding = 2) uniform DirectionalLightsData
// {
//     uint LightCount;
//     DirectionalLight Lights[1000];
// } u_DirectionalLights;

// layout(std140, binding = 3) uniform PointLightsData
// {
//     uint LightCount;
//     PointLight Lights[1000];
// } u_PointLights;

// layout(std140, binding = 4) uniform SpotLightsData
// {
//     uint LightCount;
//     SpotLight Lights[1000];
// } u_SpotLights;

void GetDirectionalLightInfo( const in DirectionalLight directionalLight, out IncidentLight light )
{
    light.Color = directionalLight.Color;
    light.Direction = normalize(directionalLight.Direction);
    light.Visible = true;
}