struct MaterialInputs {
    vec4 BaseColor;
    float Roughness;
    float Metallic;
    float Reflectance;
    float AmbientOcclusion;
    vec4 Emissive;

#if defined(MATERIAL_HAS_NORMAL)
    vec3 normal;
#endif
};

void InitMaterial(out MaterialInputs material) {
    material.BaseColor = vec4(1.0);
    material.Roughness = 1.0;
    material.Metallic = 0.0;
    material.Reflectance = 0.5;
    material.AmbientOcclusion = 1.0;
    material.Emissive = vec4(vec3(1.0), 1.0);
#if defined(MATERIAL_HAS_NORMAL)
    material.normal = vec3(0.0, 0.0, 1.0);
#endif
}

vec4 EvaluateMaterial(const MaterialInputs material) {
    //    PixelParams pixel;
    //    GetPixelParams(material, pixel);

    //    vec4 color = EvaluateLights(material, pixel);
    //    AddEmissive(material, color);
    //    return color;
    return vec4(1.0);
}