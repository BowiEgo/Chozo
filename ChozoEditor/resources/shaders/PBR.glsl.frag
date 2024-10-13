#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
} m_Params;

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

layout(binding = 0) uniform sampler2D u_PositionTex;
layout(binding = 1) uniform sampler2D u_NormalTex;
layout(binding = 2) uniform sampler2D u_AlbedoTex;
layout(binding = 3) uniform sampler2D u_MaterialPropTex;
layout(binding = 4) uniform samplerCube u_IrradianceMap;
layout(binding = 5) uniform samplerCube u_PrefilterMap;
layout(binding = 6) uniform sampler2D u_BRDFLutTex;

const float PI = 3.14159265359;
const float Epsilon = 0.00001;
// ----------------------------------------------------------------------------
// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NdfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GaSchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return GaSchlickG1(cosLi, k) * GaSchlickG1(NdotV, k);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 CalculateDirLights(vec3 F0)
{
	vec3 result = vec3(0.0);
	for (int i = 0; i < 1; i++) //Only one light for now
	{
		if (u_Scene.DirectionalLights.Intensity == 0.0)
			continue;

		vec3 Li = u_Scene.DirectionalLights.Direction;
		vec3 Lh = normalize(Li + m_Params.View);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(m_Params.Normal, Li));
		float cosLh = max(0.0, dot(m_Params.Normal, Lh));

		vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, m_Params.View)), m_Params.Roughness);
		float D = NdfGGX(cosLh, m_Params.Roughness);
		float G = GaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

		vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
		vec3 diffuseBRDF = kd * m_Params.Albedo;

		// Cook-Torrance
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);
		specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
		result += (diffuseBRDF + specularBRDF) * cosLi * u_Scene.DirectionalLights.Color * u_Scene.DirectionalLights.Intensity * 0.01;
	}
	return result;
}
// ----------------------------------------------------------------------------
vec3 CalculatePointLights(in vec3 F0, vec3 worldPos)
{
    vec3 result = vec3(0.0);
    for(int i = 0; i < min(u_PointLights.LightCount, 1000); ++i)
    {
        // calculate per-light radiance
        PointLight light = u_PointLights.Lights[i];
		vec3 Li = normalize(light.Position - worldPos);
		float lightDistance = length(light.Position - worldPos);
		vec3 Lh = normalize(Li + m_Params.View);

        float attenuation = 1.0 / (lightDistance * lightDistance);
        vec3 Lradiance     = u_PointLights.Lights[i].Color * attenuation;

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(m_Params.Normal, Li));
		float cosLh = max(0.0, dot(m_Params.Normal, Lh));

		vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, m_Params.View)), m_Params.Roughness);
		float D = NdfGGX(cosLh, m_Params.Roughness);
		float G = GaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

		vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
		vec3 diffuseBRDF = kd * m_Params.Albedo;

		// Cook-Torrance
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);
		specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
		result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }
    return result;
}

// ----------------------------------------------------------------------------
vec3 IBL(vec3 F0, vec3 Lr)
{
    vec3 irradiance = texture(u_IrradianceMap, m_Params.Normal).rgb;
	vec3 F = FresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
	vec3 diffuseIBL = m_Params.Albedo * irradiance;

    const float MAX_REFLECTION_LOD = 4.0;
	float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
    vec3 specularIrradiance = textureLod(u_PrefilterMap, R,  m_Params.Roughness * MAX_REFLECTION_LOD).rgb;    
	//specularIrradiance = vec3(Convert_sRGB_FromLinear(specularIrradiance.r), Convert_sRGB_FromLinear(specularIrradiance.g), Convert_sRGB_FromLinear(specularIrradiance.b));

	// Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
    vec2 specularBRDF  = texture(u_BRDFLutTex, vec2(m_Params.NdotV, m_Params.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

void main()
{
    vec3 gPosition = texture(u_PositionTex, v_TexCoord).rgb;

    float ao = 1.0;

    vec4 materialProps = texture(u_MaterialPropTex, v_TexCoord);

    m_Params.Albedo    = texture(u_AlbedoTex, v_TexCoord).rgb;
    m_Params.Metalness  = materialProps.r;
    m_Params.Roughness = materialProps.g;
    m_Params.Normal = normalize(texture(u_NormalTex, v_TexCoord).rgb);
    m_Params.View = normalize(u_Scene.CameraPosition - gPosition);
    m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);
    // ao        = texture(aoMap, v_TexCoord).r;

    vec3 R = reflect(-m_Params.View, m_Params.Normal);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, m_Params.Albedo, m_Params.Metalness);

    // reflectance equation
    vec3 dirLights = CalculateDirLights(F0);
    vec3 pointLights = CalculatePointLights(F0, gPosition);

    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.1) * m_Params.Albedo * ao;

	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

    ambient = IBL(F0, Lr) * ao;

    vec3 color = ambient + dirLights + pointLights;

    // HDR tonemapping
    // color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    float alpha = 1.0;
    if (m_Params.Normal == vec3(0.0))
        discard;
    o_Color = vec4(color, alpha);
}