struct CameraData
{
    float4x4 View;
    float4x4 Projection;
};

[[vk::binding(0, 0)]] ConstantBuffer<CameraData> u_Camera;