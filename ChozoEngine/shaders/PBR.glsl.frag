#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

layout(std140, binding = 0) uniform CameraData
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    mat4 u_InverseViewProjectionMatrix;
};

#include "Includes/GBuffer.glsl"
#include "Includes/Lighting.glsl"
#include "Includes/Material.glsl"
#include "Includes/Dithering.glsl"

vec4 sRGBTransferOETF(in vec4 value) {
    return vec4(mix(pow(value.rgb, vec3(0.41666)) * 1.055 - vec3(0.055), value.rgb * 12.92, vec3(lessThanEqual(value.rgb, vec3(0.0031308)))), value.a);
}

vec4 LinearTosRGB(in vec4 value) {
    return sRGBTransferOETF(value);
}

void main()
{
    GBufferData GBuffer;
    InitGBuffer(GBuffer);

    //    BRDFContext BRDFCtx;
    //    InitBRDFContext(GBuffer, BRDFCtx);

    PhysicalMaterial material;
    InitPhysicalMaterial(GBuffer, material);

    vec3 color = vec3(0.0);

    vec3 totalEmissiveRadiance = GBuffer.Emissive;

    vec3 light = EvaluateLights(GBuffer, material);

    color += light;
    color += totalEmissiveRadiance;
    //    color = Dithering(color);

    float alpha = 1.0;

    //    color = color / (color + vec3(1.0));
    //    color = pow(color, vec3(1.0/2.2));

    color = LinearTosRGB(vec4(color, 1.0)).rgb;
    o_Color = vec4(color, 1.0);
}