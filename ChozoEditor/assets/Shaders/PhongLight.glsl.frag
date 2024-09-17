#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

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

// layout(push_constant) uniform FragUniforms
// {
//     vec3 AmbientColor;
//     vec3 DiffuseColor;
//     vec3 SpecularColor;
//     float AmbientStrength;
//     float Metalness;
//     float Roughness;
// } u_Material;

struct Material {
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Metalness;
    float Roughness;
};

layout(binding = 0) uniform sampler2D u_PositionTex;
layout(binding = 1) uniform sampler2D u_NormalTex;
layout(binding = 2) uniform sampler2D u_AmbientTex;
layout(binding = 3) uniform sampler2D u_DiffuseTex;
layout(binding = 4) uniform sampler2D u_SpecularTex;
layout(binding = 5) uniform sampler2D u_MetalnessTex;
layout(binding = 6) uniform sampler2D u_RoughnessTex;

float Constant = 1.0;
float Linear = 0.09f;
float Quadratic = 0.032f;

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, Material material)
{
    // if (!light.enable)
    // {
    //     return vec3(0.0);
    // }

    vec3 lightDir = normalize(-light.Direction);
    vec3 halfDir = normalize(lightDir + viewDir);

    
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * material.Diffuse;

    float shininess = (1.01 - material.Roughness) * 128.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);

    float fresnelFactor = pow(1.0 - max(dot(viewDir, normal), 0.0), 5.0);
    vec3 specular = mix(material.Specular, vec3(1.0), fresnelFactor);

    // Final result
    // vec3 ambient  = light.Ambient  * texturedDiffuse;
    // vec3 diffuse  = light.Diffuse  * diff * texturedDiffuse;
    // vec3 specular = light.Specular * spec * texturedSpecular;

    vec3 result = mix(diffuse, specular, material.Metalness) * light.Color * light.Intensity;
    result += specular * spec * (1.0 - material.Metalness);

    return result;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, Material material)
{
    // if (!light.enable)
    // {
    //     return vec3(0.0);
    // }

    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * material.Diffuse;

    float shininess = (1.01 - material.Roughness) * 128.0;
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = spec * material.Specular;

    float distance    = length(light.Position - fragPos);
    float attenuation = 1.0 / (Constant + Linear * distance + 
                        Quadratic * (distance * distance));

    // Final result
    // vec3 ambient  = light.ambient  * texturedDiffuse;
    // vec3 diffuse  = light.diffuse  * diff * texturedDiffuse;
    // vec3 specular = light.specular * spec * texturedSpecular;
    // ambient  *= attenuation;
    // diffuse  *= attenuation;
    // specular *= attenuation;

    vec3 result = mix(diffuse, specular, material.Metalness) * attenuation;
    result += specular * spec * (1.0 - material.Metalness);
    result *= light.Color * light.Intensity;

    return result;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    vec3 spotDir = normalize(light.Direction);
    // vec3 spotDir = normalize(-vec3(-1.0, 0.0, 0.0));

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * material.Diffuse;

    float shininess = (1.01 - material.Roughness) * 128.0;
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = spec * material.Specular;

    float distance = length(light.Position - fragPos);
    float theta = dot(lightDir, -spotDir);
    float outerCutOff = cos(radians(light.Angle + light.AngleAttenuation + 0.01));
    float epsilon = cos(radians(light.Angle)) - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    float attenuation = 1.0 / (Constant + Linear * distance + 
                        Quadratic * (distance * distance));
    attenuation *= intensity;
    
    vec3 result = mix(diffuse, specular, material.Metalness) * attenuation;
    result += specular * spec * (1.0 - material.Metalness);
    result *= light.Color * light.Intensity;
    return result;
}

void main()
{
    vec3 directionalLight = vec3(0.0);
    vec3 pointLights = vec3(0.0);
    vec3 spotLights = vec3(0.0);
    vec3 finalLight = vec3(0.0);

    vec3 gPosition = texture(u_PositionTex, v_TexCoord).rgb;
    vec3 gNormal = texture(u_NormalTex, v_TexCoord).rgb;
    vec3 gAmbient = texture(u_AmbientTex, v_TexCoord).rgb;
    vec3 gDiffuse = texture(u_DiffuseTex, v_TexCoord).rgb;
    vec3 gSpecular = texture(u_SpecularTex, v_TexCoord).rgb;
    float gMetalness = texture(u_MetalnessTex, v_TexCoord).r;
    float gRoughness = texture(u_RoughnessTex, v_TexCoord).r;

    Material material;
    material.Diffuse = gDiffuse;
    material.Specular = gSpecular;
    material.Metalness = gMetalness;
    material.Roughness = gRoughness;

    vec3 viewDirection = normalize(u_Scene.CameraPosition - gPosition);
    vec3 normal = normalize(gNormal);

    directionalLight += calcDirLight(u_Scene.DirectionalLights, normal, viewDirection, material);

    for(int i = 0; i < u_PointLights.LightCount; i++)
        pointLights += calcPointLight(u_PointLights.Lights[i], normal, gPosition, viewDirection, material);

    for(int i = 0; i < u_SpotLights.LightCount; i++)
        spotLights += calcSpotLight(u_SpotLights.Lights[i], normal, gPosition, viewDirection, material);

    finalLight = gAmbient + directionalLight + pointLights + spotLights;

    o_Color = vec4(finalLight, 1.0);
}