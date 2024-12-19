#include "./Varyings.glsl"
#include "./SceneData.glsl"

layout(binding = 0) uniform sampler2D u_PositionTex;
layout(binding = 1) uniform sampler2D u_NormalTex;
layout(binding = 2) uniform sampler2D u_BaseColorTex;
layout(binding = 3) uniform sampler2D u_MaterialPropTex;

struct GBufferData
{
    vec3 BaseColor;
    float PerceptualRoughness;
    float Roughness;
    float Metallic;
    float Reflectance;

    float AO;
    highp vec3 Position;
    vec3 Normal;
    vec3 View;
    vec3 Reflected;
} GBuffer;

void InitGBuffer(out GBufferData GBuffer)
{
    vec4 materialProps = texture(u_MaterialPropTex, v_TexCoord);

    GBuffer.BaseColor           = texture(u_BaseColorTex, v_TexCoord).rgb;
    GBuffer.Metallic            = materialProps.r;
    GBuffer.PerceptualRoughness = max(materialProps.g, 0.001);
    GBuffer.Roughness           = GBuffer.PerceptualRoughness * GBuffer.PerceptualRoughness;
    GBuffer.Reflectance         = 0.5;

//    GBuffer.EnergyCompensation = vec3(1.0);
    //    GBuffer.EnergyCompensation = 1.0 + f0 * (1.0 / dfg.y - 1.0);

    GBuffer.AO        = 1.0;
    GBuffer.Position  = texture(u_PositionTex, v_TexCoord).rgb;
    GBuffer.Normal    = normalize(texture(u_NormalTex, v_TexCoord).rgb);
    GBuffer.View      = normalize(u_Scene.CameraPosition - GBuffer.Position);
//    GBuffer.Reflected = reflect(-GBuffer.View, GBuffer.Normal);
    GBuffer.Reflected = 2.0 * dot(GBuffer.View, GBuffer.Normal) * GBuffer.Normal - GBuffer.View;
}