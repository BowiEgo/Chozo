#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

layout(binding = 0) uniform sampler2D u_PositionMap;
layout(binding = 1) uniform sampler2D u_NormalMap;
layout(binding = 2) uniform sampler2D u_BaseColorMap;
layout(binding = 3) uniform sampler2D u_MaterialPropMap;

#include "Snippets/Fragment/Scene.glsl"
#include "Snippets/Fragment/Light.glsl"

struct Material {
    vec3 Diffuse;
    float Metallic;
    float Roughness;
    float Ambient;
    float Specular;
};

float Constant = 1.0;
float Linear = 0.09f;
float Quadratic = 0.032f;

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(-light.Direction);
    vec3 halfDir = normalize(lightDir + viewDir);
    
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * material.Diffuse;

    float shininess = (1.01 - material.Roughness) * 128.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);

    float fresnelFactor = pow(1.0 - max(dot(viewDir, normal), 0.0), 5.0);
    vec3 specular = mix(vec3(material.Specular), vec3(1.0), fresnelFactor);

    vec3 result = mix(diffuse, specular, material.Metallic) * light.Color * light.Intensity;
    result += specular * spec * (1.0 - material.Metallic);

    return result;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * material.Diffuse;

    float shininess = (1.01 - material.Roughness) * 128.0;
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = spec * vec3(material.Specular);

    float distance    = length(light.Position - fragPos);
    float attenuation = 1.0 / (Constant + Linear * distance + 
                        Quadratic * (distance * distance));

    vec3 result = mix(diffuse, specular, material.Metallic) * attenuation;
    result += specular * spec * (1.0 - material.Metallic);
    result *= light.Color * light.Intensity;

    return result;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    vec3 spotDir = normalize(light.Direction);

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * material.Diffuse;

    float shininess = (1.01 - material.Roughness) * 128.0;
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = spec * vec3(material.Specular);

    float distance = length(light.Position - fragPos);
    float theta = dot(lightDir, -spotDir);
    float outerCutOff = cos(radians(light.Angle + light.AngleAttenuation + 0.01));
    float epsilon = cos(radians(light.Angle)) - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    float attenuation = 1.0 / (Constant + Linear * distance + 
                        Quadratic * (distance * distance));
    attenuation *= intensity;
    
    vec3 result = mix(diffuse, specular, material.Metallic) * attenuation;
    result += specular * spec * (1.0 - material.Metallic);
    result *= light.Color * light.Intensity;
    return result;
}

void main()
{
    vec3 directionalLight = vec3(0.0);
    vec3 pointLights = vec3(0.0);
    vec3 spotLights = vec3(0.0);
    vec3 finalLight = vec3(0.0);

    vec3 gPosition = texture(u_PositionMap, v_TexCoord).rgb;
    vec3 gNormal = texture(u_NormalMap, v_TexCoord).rgb;
    vec3 gDiffuse = texture(u_BaseColorMap, v_TexCoord).rgb;
    vec4 gMaterialProps = texture(u_MaterialPropMap, v_TexCoord);

    float gMetallic = gMaterialProps.r;
    float gRoughness = gMaterialProps.g;
    float gAmbient = gMaterialProps.b;
    float gSpecular = gMaterialProps.a;

    Material material;
    material.Diffuse = gDiffuse;
    material.Specular = gSpecular;
    material.Metallic = gMetallic;
    material.Roughness = gRoughness;

    vec3 viewDirection = normalize(u_Scene.CameraPosition - gPosition);
    vec3 normal = normalize(gNormal);

    directionalLight += calcDirLight(u_DirectionalLights.Lights[0], normal, viewDirection, material);

    for(int i = 0; i < u_PointLights.LightCount; i++)
        pointLights += calcPointLight(u_PointLights.Lights[i], normal, gPosition, viewDirection, material);

    for(int i = 0; i < u_SpotLights.LightCount; i++)
        spotLights += calcSpotLight(u_SpotLights.Lights[i], normal, gPosition, viewDirection, material);

    finalLight = vec3(gAmbient) + directionalLight + pointLights + spotLights;

    o_Color = vec4(finalLight, 1.0);
}