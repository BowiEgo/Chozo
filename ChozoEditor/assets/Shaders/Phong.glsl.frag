#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_FragPosition;
layout(location = 3) in flat int v_EntityID;

struct DirectionalLight
{
	vec3 Direction;
    vec3 Color;
	float Intensity;
};

layout(std140, binding = 1) uniform SceneData
{
	DirectionalLight DirectionalLights;
	vec3 CameraPosition; // Offset = 32
	float EnvironmentMapIntensity;
};

layout(push_constant) uniform FragUniforms
{
    vec3 AmbientColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
    float AmbientStrength;
    float Metalness;
    float Roughness;
} u_Material;

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    // if (!light.enable)
    // {
    //     return vec3(0.0);
    // }

    vec3 lightDir = normalize(-light.Direction);
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 ambient  = u_Material.AmbientColor * u_Material.AmbientStrength;
    
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * u_Material.DiffuseColor;

    float shininess = (1.01 - u_Material.Roughness) * 128.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);

    float fresnelFactor = pow(1.0 - max(dot(viewDir, normal), 0.0), 5.0);
    vec3 specular = mix(u_Material.SpecularColor, vec3(1.0), fresnelFactor);

    // Final result
    // vec3 ambient  = light.Ambient  * texturedDiffuse;
    // vec3 diffuse  = light.Diffuse  * diff * texturedDiffuse;
    // vec3 specular = light.Specular * spec * texturedSpecular;

    vec3 finalColor = mix(diffuse, specular, u_Material.Metalness) * light.Color * light.Intensity;
    finalColor += specular * spec * (1.0 - u_Material.Metalness);
    finalColor += ambient;

    return finalColor;
}

void main()
{
    vec3 finalLight = vec3(0.0);
    vec3 viewDirection = normalize(CameraPosition - v_FragPosition);

    vec3 directionalLight = calcDirLight(DirectionalLights, normalize(v_Normal), viewDirection);

    finalLight = directionalLight;
    o_Color = vec4(finalLight, 1.0);
    o_EntityID = v_EntityID;
}