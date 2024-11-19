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