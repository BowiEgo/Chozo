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

#include "shaders://Camera.glsl"

layout(set = 1, binding = 0) uniform SceneUBO
{
    vec3 CameraPosition;
} u_Scene;

layout(set = 1, binding = 1) uniform sampler2D u_PositionMap;
layout(set = 1, binding = 2) uniform sampler2D u_NormalMap;
layout(set = 1, binding = 3) uniform sampler2D u_BaseColorMap;
layout(set = 1, binding = 4) uniform sampler2D u_RMAOMap;
layout(set = 1, binding = 5) uniform sampler2D u_EmissiveMap;
layout(set = 1, binding = 6) uniform sampler2D u_DepthMap;
layout(set = 1, binding = 7) uniform sampler2D u_BRDFLutTex;
layout(set = 1, binding = 8) uniform samplerCube u_IrradianceCubeMap;
layout(set = 1, binding = 9) uniform samplerCube u_PrefilteredCubeMap;

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
    // color = Dithering(color);

    float alpha = 1.0;

    // color = color / (color + vec3(1.0));
    // color = pow(color, vec3(1.0/2.2));

    color = LinearTosRGB(vec4(color, 1.0)).rgb;
    o_Color = vec4(color, 1.0);
}

#endif
