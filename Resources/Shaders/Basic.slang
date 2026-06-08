#include "./Camera.slang"

struct VertexConstant
{
    float4x4 ModelMatrix;
    float3x3 NormalMatrix;
};
[[vk::push_constant]] VertexConstant u_VertContant;

struct VSInput
{
    [[vk::location(0)]] float3 a_Position;
    [[vk::location(1)]] float3 a_Normal;
    [[vk::location(2)]] float2 a_TexCoord;
    [[vk::location(3)]] float3 a_Tangent;
    [[vk::location(4)]] float3 a_Bitangent;
};

struct VSOutput
{
    float4 position : SV_Position;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    float4x4 model = u_VertContant.ModelMatrix;
    float4x4 mvp = mul(u_Camera.Projection, mul(u_Camera.View, model));

    output.position = mul(mvp, float4(input.a_Position, 1.0));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float3 finalColor = float3(0.5, 0.5, 0.0);
    return float4(finalColor, 1.0);
}