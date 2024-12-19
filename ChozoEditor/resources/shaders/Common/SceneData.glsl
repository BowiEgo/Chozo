layout(std140, binding = 1) uniform SceneData
{
    vec3 CameraPosition; // Offset = 32
    float EnvironmentMapIntensity;
} u_Scene;