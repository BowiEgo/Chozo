#version 450

layout(location = 0) out vec4 o_Color;

#include "Includes/GBuffer.glsl"
#include "Includes/BRDF.glsl"
#include "Includes/PBRLighting.glsl"

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

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    o_Color = vec4(color, alpha);
}