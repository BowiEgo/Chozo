vec4 EvaluateLights(const GBufferData GBuffer, out vec3 color) {

	//    color += EvaluateIBL(pixel);
	color += EvaluateDirectionalLight(GBuffer);
	//    color += EvaluatePunctualLights(pixel);
}

vec3 EvaluateDirectionalLight(const GBufferData GBuffer, const DirectionalLight light)
{
	vec3 h = normalize(GBuffer.View + light.);

	float NoV = shading_NoV;
	float NoL = Saturate(light.NoL);
	float NoH = Saturate(dot(shading_normal, h));
	float LoH = Saturate(dot(light.l, h));

	float D = D_GGX(NoH, pixel.roughness);
	vec3  F = F_Schlick(LoH, f0);
	float V = V_SmithGGXCorrelated(NoV, NoL, pixel.roughness);

	vec3 Fr = (D * V) * F;
	vec3 Fd = pixel.diffuseColor * Fd_Lambert();

	vec3 color = Fd + Fr * pixel.energyCompensation;

	DirectionalLight light = u_Scene.directionalLight;
	float occlusion = 1.0; // visibility
	return (color * light.color) *
	(light.intensity * 1.0 * NoL * occlusion);
}
// ----------------------------------------------------------------------------
vec3 CalculateDirLights(const GBufferData GBuffer, const BRDFContextData BRDFContext, vec3 F0)
{
	vec3 result = vec3(0.0);
	for(int i = 0; i < min(u_DirectionalLights.LightCount, 1000); ++i)
	{
		DirectionalLight light = u_DirectionalLights.Lights[i];

		if (light.Intensity == 0.0)
			continue;

		vec3 Li = light.Direction;
		vec3 Lh = normalize(Li + GBuffer.View);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(GBuffer.Normal, Li));
		float cosLh = max(0.0, dot(GBuffer.Normal, Lh));

		vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, GBuffer.View)), GBuffer.Roughness);
		float D = NdfGGX(cosLh, GBuffer.Roughness);
		float G = GaSchlickGGX(cosLi, BRDFContext.NdotV, GBuffer.Roughness);

		vec3 kd = (1.0 - F) * (1.0 - GBuffer.Metallic);
		vec3 diffuseBRDF = kd * GBuffer.BaseColor;

		// Cook-Torrance
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * BRDFContext.NdotV);
		specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
		result += (diffuseBRDF + specularBRDF) * cosLi * light.Color * light.Intensity * 0.01;
	}
	return result;
}
// ----------------------------------------------------------------------------
vec3 CalculatePointLights(const GBufferData GBuffer, const BRDFContextData BRDFContext, in vec3 F0)
{
    vec3 result = vec3(0.0);
    for(int i = 0; i < min(u_PointLights.LightCount, 1000); ++i)
    {
        // calculate per-light radiance
        PointLight light = u_PointLights.Lights[i];
		vec3 Li = normalize(light.Position - GBuffer.Position);
		float lightDistance = length(light.Position - GBuffer.Position);
		vec3 Lh = normalize(Li + GBuffer.View);

        float attenuation = 1.0 / (lightDistance * lightDistance);
        vec3 Lradiance     = u_PointLights.Lights[i].Color * attenuation;

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(GBuffer.Normal, Li));
		float cosLh = max(0.0, dot(GBuffer.Normal, Lh));

		vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, GBuffer.View)), GBuffer.Roughness);
		float D = NdfGGX(cosLh, GBuffer.Roughness);
		float G = GaSchlickGGX(cosLi, BRDFContext.NdotV, GBuffer.Roughness);

		vec3 kd = (1.0 - F) * (1.0 - GBuffer.Metallic);
		vec3 diffuseBRDF = kd * GBuffer.BaseColor;

		// Cook-Torrance
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * BRDFContext.NdotV);
		specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
		result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }
    return result;
}