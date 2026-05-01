#include "shaders://Includes/GBuffer.glsl"
#include "shaders://Includes/Material.glsl"
#include "shaders://Includes/LightInfo.glsl"
#include "shaders://Includes/LightRE.glsl"

/**
 * This function evaluates all lights one by one:
 * - Image based lights (IBL)
 * - Directional lights
 * - Punctual lights
 *
 * Area lights are currently not supported.
 *
 * Returns a pre-exposed HDR RGBA color in linear space.
 */
vec3 EvaluateLights(const in GBufferData GBuffer, const in PhysicalMaterial material)
{
    vec3 worldNormal = GBuffer.PerturbedNormal;
    vec3 geometryPosition = GBuffer.Position;
    vec3 geometryNormal = normalize((vec4(worldNormal, 0.0) * inverse(u_Camera.ViewMatrix)).xyz);
    vec3 geometryViewDir = GBuffer.View;
    vec3 geometryClearcoatNormal = vec3(0.0);

    //    color += EvaluateIBL(GBuffer, BRDFContext);
    ReflectedLight reflectedLight = ReflectedLight(vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0));

    // Direct lights
    // IncidentLight directLight;

    // for (int i = 0; i < u_DirectionalLights.LightCount; i++)
    // {
    //     DirectionalLight directionalLight = u_DirectionalLights.Lights[i];
    //     GetDirectionalLightInfo(directionalLight, directLight);
    //     RE_Direct_Physical(
    //         directLight,
    //         geometryPosition,
    //         geometryNormal,
    //         geometryViewDir,
    //         geometryClearcoatNormal,
    //         material,
    //         reflectedLight
    //     );
    // }

    // Indirect lights

    // Diffuse
    vec3 iblIrradiance = vec3(0.0);
    // vec3 irradiance = u_Scene.AmbientLightColor;
    vec3 irradiance = vec3(0.0);
    iblIrradiance += GetIBLIrradiance(geometryNormal, 4.0);
    RE_IndirectDiffuse_Physical(
        irradiance,
        geometryPosition,
        geometryNormal,
        geometryViewDir,
        geometryClearcoatNormal,
        material,
        reflectedLight
    );

    // Specular
    vec3 radiance = vec3(0.0);
    vec3 clearcoatRadiance = vec3(0.0);

    vec3 r = 2.0 * dot(GBuffer.View, GBuffer.PerturbedNormal) * GBuffer.PerturbedNormal - GBuffer.View;
    radiance += GetIBLRadiance(geometryNormal, geometryViewDir, material.Roughness);

    RE_IndirectSpecular_Physical(
        radiance,
        iblIrradiance,
        clearcoatRadiance,
        geometryPosition,
        geometryNormal,
        geometryViewDir,
        geometryClearcoatNormal,
        material,
        reflectedLight
    );

    vec3 totalDiffuse = reflectedLight.DirectDiffuse + reflectedLight.IndirectDiffuse;
    vec3 totalSpecular = reflectedLight.DirectSpecular + reflectedLight.IndirectSpecular;

    //    DebugColor = totalSpecular;
    return totalDiffuse + totalSpecular;
}