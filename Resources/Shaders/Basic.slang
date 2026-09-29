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
    // CPU-side matrices are row-vector (v * M) convention: translations live in the last ROW.
    // Multiplying as M * v transposed every transform, so w' lost its constant term and the
    // geometry was projected far off-screen. Chain as v * model * view * projection instead.
    float4x4 mvp = mul(mul(model, u_Camera.View), u_Camera.Projection);

    output.position = mul(float4(input.a_Position, 1.0), mvp);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float3 finalColor = float3(0.5, 0.5, 0.0);
    return float4(finalColor, 1.0);
}