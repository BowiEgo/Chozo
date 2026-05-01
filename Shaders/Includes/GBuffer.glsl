struct GBufferData
{
    vec3 BaseColor;
    float PerceptualRoughness;
    float Roughness;
    float Metallic;
    float Reflectance;
    vec3 Emissive;

    float AO;
    vec3 Position;
    vec3 PerturbedNormal;
    vec3 View;
    vec3 Reflected;
};

void InitGBuffer(out GBufferData GBuffer)
{
    vec4 RMAO = texture(u_RMAOMap, v_TexCoord);

    GBuffer.BaseColor           = texture(u_BaseColorMap, v_TexCoord).rgb;
    GBuffer.PerceptualRoughness = max(RMAO.r, 0.001);
    GBuffer.Roughness           = GBuffer.PerceptualRoughness * GBuffer.PerceptualRoughness;
    GBuffer.Metallic            = RMAO.g;
    GBuffer.Reflectance         = RMAO.b;
    GBuffer.Emissive            = texture(u_EmissiveMap, v_TexCoord).rgb;

//    GBuffer.EnergyCompensation = vec3(1.0);
    //    GBuffer.EnergyCompensation = 1.0 + f0 * (1.0 / dfg.y - 1.0);

    GBuffer.AO        = 1.0;
    GBuffer.Position  = texture(u_PositionMap, v_TexCoord).rgb;
    GBuffer.PerturbedNormal    = normalize(texture(u_NormalMap, v_TexCoord).rgb);
    GBuffer.View      = normalize(u_Scene.CameraPosition - GBuffer.Position);
//    GBuffer.Reflected = reflect(-GBuffer.View, GBuffer.PerturbedNormal);
    GBuffer.Reflected = 2.0 * dot(GBuffer.View, GBuffer.PerturbedNormal) * GBuffer.PerturbedNormal - GBuffer.View;
}