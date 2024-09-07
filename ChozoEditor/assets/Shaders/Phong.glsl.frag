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
	float Multiplier;
};

layout(std140, binding = 1) uniform SceneData
{
	DirectionalLight DirectionalLights;
	vec3 CameraPosition; // Offset = 32
	float EnvironmentMapIntensity;
};

layout(push_constant) uniform FragUniforms
{
    vec4 Color;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Shininess;
} u_Material;

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    // if (!light.enable)
    // {
    //     return vec3(0.0);
    // }

    vec3 lightDir = normalize(-light.Direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.Shininess);

    // Final result
    // vec3 ambient  = light.Ambient  * texturedDiffuse;
    // vec3 diffuse  = light.Diffuse  * diff * texturedDiffuse;
    // vec3 specular = light.Specular * spec * texturedSpecular;

    vec3 ambient  = u_Material.Ambient;
    vec3 diffuse  = diff * u_Material.Diffuse;
    vec3 specular = spec * u_Material.Specular;
    return ambient + diffuse + specular;
}

void main()
{
    vec3 finalLight = vec3(0.0);
    vec4 texColor = u_Material.Color;
    vec3 viewDirection = normalize(CameraPosition - v_FragPosition);

    vec3 directionalLight = calcDirLight(DirectionalLights, v_Normal, viewDirection);

    finalLight = directionalLight;
    o_Color = vec4(finalLight * u_Material.Color.rbg, u_Material.Color.a);
    o_EntityID = v_EntityID;
}