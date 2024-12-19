#version 450
//
layout(location = 0) out vec4 o_Color;
//
//layout(location = 0) in vec2 v_TexCoord;
//layout(location = 1) in vec3 v_FragPosition;
//
////#include "Common/SceneData.glsl"
////#include "Common/Lighting.glsl"
//
//layout(binding = 0) uniform sampler2D u_PositionTex;
//layout(binding = 1) uniform sampler2D u_NormalTex;
//layout(binding = 2) uniform sampler2D u_BaseColorTex;
//layout(binding = 3) uniform sampler2D u_MaterialPropTex;
//layout(binding = 4) uniform samplerCube u_IrradianceMap;
//layout(binding = 5) uniform samplerCube u_PrefilterMap;
//layout(binding = 6) uniform sampler2D u_BRDFLutTex;
//
//struct PBRParameters
//{
//    vec3 BaseColor;
//    float Roughness;
//    float Metallic;
//
//    vec3 Normal;
//    vec3 View;
//    float NdotV;
//} m_Params;
//
//struct MaterialInputs {
//    vec4 BaseColor;
//    float Roughness;
//    float Metallic;
//    float Reflectance;
//    float AmbientOcclusion;
//    vec4 Emissive;
//
//#if defined(MATERIAL_HAS_NORMAL)
//    vec3 normal;
//#endif
//};
//
//void InitMaterial(out MaterialInputs material) {
//    material.BaseColor = vec4(1.0);
//    material.Roughness = 1.0;
//    material.Metallic = 0.0;
//    material.Reflectance = 0.5;
//    material.AmbientOcclusion = 1.0;
//    material.Emissive = vec4(vec3(1.0), 1.0);
//#if defined(MATERIAL_HAS_NORMAL)
//    material.normal = vec3(0.0, 0.0, 1.0);
//#endif
//}
//
//vec4 EvaluateMaterial(const MaterialInputs material) {
////    PixelParams pixel;
////    GetPixelParams(material, pixel);
//
////    vec4 color = EvaluateLights(material, pixel);
//    //    AddEmissive(material, color);
////    return color;
//    return vec4(1.0);
//}
//
void main() {
////    MaterialInputs inputs;
////    InitMaterial(inputs);
////
////    m_Params.Normal = normalize(texture(u_NormalTex, v_TexCoord).rgb);
////    float alpha = 1.0;
////    if (m_Params.Normal == vec3(0.0))
////        discard;
////
////    //    // Invoke user custom code
////    //    material(inputs);
////
////    o_Color = EvaluateMaterial(inputs);
    o_Color = vec4(vec3(1.0), 1.0);
}