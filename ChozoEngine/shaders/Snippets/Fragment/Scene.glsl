layout(std140, binding = 1) uniform SceneData
{
    vec3 CameraPosition;
    float EnvironmentMapIntensity;
    vec3 AmbientLightColor;
} u_Scene;