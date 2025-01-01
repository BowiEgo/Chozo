#include "../Snippets/Fragment/Varyings.glsl"
#include "../Snippets/Fragment/Scene.glsl"

layout(binding = 0) uniform sampler2D u_PositionMap;
layout(binding = 1) uniform sampler2D u_NormalMap;
layout(binding = 2) uniform sampler2D u_BaseColorMap;
layout(binding = 3) uniform sampler2D u_MaterialPropMap;

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
    vec4 materialPropsTex = texture(u_MaterialPropMap, v_TexCoord);

    GBuffer.BaseColor           = texture(u_BaseColorMap, v_TexCoord).rgb;
    GBuffer.Metallic            = materialPropsTex.r;
    GBuffer.PerceptualRoughness = max(materialPropsTex.g, 0.001);
    GBuffer.Roughness           = GBuffer.PerceptualRoughness * GBuffer.PerceptualRoughness;
    GBuffer.Reflectance         = 0.4;

//    GBuffer.EnergyCompensation = vec3(1.0);
    //    GBuffer.EnergyCompensation = 1.0 + f0 * (1.0 / dfg.y - 1.0);

    GBuffer.AO        = 1.0;
    GBuffer.Position  = texture(u_PositionMap, v_TexCoord).rgb;
    GBuffer.Normal    = normalize(texture(u_NormalMap, v_TexCoord).rgb);
    GBuffer.View      = normalize(u_Scene.CameraPosition - GBuffer.Position);
//    GBuffer.Reflected = reflect(-GBuffer.View, GBuffer.Normal);
    GBuffer.Reflected = 2.0 * dot(GBuffer.View, GBuffer.Normal) * GBuffer.Normal - GBuffer.View;
}