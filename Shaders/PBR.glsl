#version 460

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 v_TexCoord;

void main() {
    v_TexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(v_TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

#include "shaders://Includes/GBuffer.glsl"
#include "shaders://Includes/Lighting.glsl"
#include "shaders://Includes/Material.glsl"
#include "shaders://Includes/Dithering.glsl"

vec4 sRGBTransferOETF(in vec4 value) {
    return vec4(mix(pow(value.rgb, vec3(0.41666)) * 1.055 - vec3(0.055), value.rgb * 12.92, vec3(lessThanEqual(value.rgb, vec3(0.0031308)))), value.a);
}

vec4 LinearTosRGB(in vec4 value) {
    return sRGBTransferOETF(value);
}

void main() {
    GBufferData GBuffer;
    InitGBuffer(GBuffer);

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

#endif
