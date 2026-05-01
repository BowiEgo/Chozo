#version 460

#ifdef VERTEX_SHADER

layout(push_constant) uniform PushConsts {
    int u_FaceIndex;
} PC;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_Direction;

vec3 GetDirection(const in int face, const in vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;

    switch (face) {
        case 0: return vec3( 1.0, -ndc.y, -ndc.x); // +X (Right)
        case 1: return vec3(-1.0, -ndc.y,  ndc.x); // -X (Left)
        case 2: return vec3( ndc.x, -1.0, -ndc.y); // +Y (Top)
        case 3: return vec3( ndc.x,  1.0,  ndc.y); // -Y (Bottom)
        case 4: return vec3( ndc.x, -ndc.y,  1.0); // +Z (Back)
        case 5: return vec3(-ndc.x, -ndc.y, -1.0); // -Z (Front)
        default: return vec3(0.0);
    }
}

void main() {
    v_Direction = normalize(GetDirection(PC.u_FaceIndex, a_TexCoord));
    gl_Position = vec4(a_Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 v_Direction;

layout(location = 0) out vec4 o_Color;

layout(set = 1, binding = 0) uniform sampler2D u_EquirectangularMap;

#include "shaders://Utils/Math.glsl"

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    float lon = atan(v.z, v.x);
    float lat = asin(v.y);
    lon -= HALF_PI;
    if (lon < 0.0) lon += 2.0 * PI;
    vec2 uv = vec2(lon / (2.0 * PI), lat / PI + 0.5);
    return uv;
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{       
    vec2 uv = SampleSphericalMap(normalize(v_Direction)); // make sure to normalize v_Direction
    vec3 hdrColor = texture(u_EquirectangularMap, uv).rgb;

    float exposure = 1.0; 
    hdrColor *= exposure;

    vec3 mappedColor = ACESFilm(hdrColor);

    vec3 color = pow(mappedColor, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
}

#endif
