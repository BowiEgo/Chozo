#version 460

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 v_TexCoord;

void main() {
    // Generate UV: (0,0), (2,0), (0,2)
    v_TexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    // Map to NDC: (-1,-1), (3,-1), (-1,3)
    gl_Position = vec4(v_TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
    // v_TexCoord.x = 1.0 - v_TexCoord.x;
    // v_TexCoord.y = 1.0 - v_TexCoord.y;
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 v_TexCoord;

layout(set = 1, binding = 0) uniform DebugUBO {
    int Mode;
} u_Debug;

layout(set = 1, binding = 1) uniform sampler2D u_PBRMap;
layout(set = 1, binding = 2) uniform sampler2D u_PositionMap;
layout(set = 1, binding = 3) uniform sampler2D u_NormalMap;
layout(set = 1, binding = 4) uniform sampler2D u_BaseColorMap;
layout(set = 1, binding = 5) uniform sampler2D u_RMAOMap;
layout(set = 1, binding = 6) uniform sampler2D u_EmissiveMap;
layout(set = 1, binding = 7) uniform sampler2D u_DepthMap;
layout(set = 1, binding = 8) uniform sampler2D u_SkyboxMap;
layout(set = 1, binding = 9) uniform samplerCube u_SkyboxCubeMap;
layout(set = 1, binding = 10) uniform samplerCube u_IrradianceCubeMap;
layout(set = 1, binding = 11) uniform samplerCube u_PrefilteredCubeMap;

layout(location = 0) out vec4 o_Color;

#include "shaders://Utils/Packing.glsl"
#include "shaders://Includes/CubemapPreview.glsl"

void main() {
    vec3 result = vec3(0.0);
    float near = 0.1;
    float far = 100.0;

    switch (u_Debug.Mode) {
        case 0: // Position
            result = texture(u_PBRMap, v_TexCoord).rgb;
            break;

        case 1: // Position
            result = texture(u_PositionMap, v_TexCoord).rgb;
            break;
            
        case 2: // Normal
            result = texture(u_NormalMap, v_TexCoord).rgb;
            break;
            
        case 3: // BaseColor
            result = texture(u_BaseColorMap, v_TexCoord).rgb;
            break;
            
        case 4: // Roughness (R)
            result = vec3(texture(u_RMAOMap, v_TexCoord).r);
            break;
            
        case 5: // Metallic (G)
            result = vec3(texture(u_RMAOMap, v_TexCoord).g);
            break;
            
        case 6: // AO (B)
            result = vec3(texture(u_RMAOMap, v_TexCoord).b);
            break;
            
        case 7: // Emissive
            result = texture(u_EmissiveMap, v_TexCoord).rgb;
            break;

        case 8: // Depth
            float d = texture(u_DepthMap, v_TexCoord).r;
            float ld = LinearizeNDCDepthToViewZ(d, near, far) / far;
            result = vec3(d);
            break;

        case 9: // Skybox
            result = texture(u_SkyboxMap, v_TexCoord).rgb;
            break;
        
        case 10:
            result = CubemapPreview(u_SkyboxCubeMap, v_TexCoord, 0).rgb;
            break;

        case 11:
            result = CubemapPreview(u_IrradianceCubeMap, v_TexCoord, 0).rgb;
            break;
            
        case 12:
            result = CubemapPreview(u_PrefilteredCubeMap, v_TexCoord, 0).rgb;
            break;
            
        default:
            result = vec3(1.0, 0.0, 1.0); // Error: Magenta
            break;
    }

    // float thickness = 0.01;
    // vec2 grid = abs(fract(v_TexCoord * 10.0 - 0.5) - 0.5) / fwidth(v_TexCoord * 10.0);
    // float line = min(grid.x, grid.y);
    // float isWireframe = 1.0 - smoothstep(0.0, 1.0, line);

    // o_Color = mix(vec4(result, 1.0), vec4(0.0, 1.0, 0.0, 1.0), isWireframe);
    // o_Color = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
    o_Color = vec4(result, 1.0);
}

#endif
