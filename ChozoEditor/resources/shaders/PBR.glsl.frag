#version 450

layout(location = 0) out vec4 o_Color;

#include "Common/GBuffer.glsl"
#include "Common/BRDF.glsl"
#include "Common/SceneData.glsl"
#include "Common/Lighting.glsl"

void main()
{
    GBufferData GBuffer;
    InitGBuffer(GBuffer);

    BRDFContext BRDFCtx;
    InitBRDFContext(GBuffer, BRDFCtx);

    vec3 color = vec3(0.0);
    vec3 light = EvaluateLights(GBuffer, BRDFCtx);

    color += light;

    float alpha = 1.0;
    if (GBuffer.Normal == vec3(0.0))
        discard;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    o_Color = vec4(color, alpha);
}