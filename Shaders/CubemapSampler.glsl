#version 460

#ifdef VERTEX_SHADER

layout(push_constant) uniform PushConsts {
    int u_FaceIndex;
} pc;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_Direction;

vec3 GetDirection(int face, vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;
    switch (face) {
        case 0: return vec3( 1.0, -ndc.y, -ndc.x); // +X
        case 1: return vec3(-1.0, -ndc.y,  ndc.x); // -X
        case 2: return vec3( ndc.x,  1.0, -ndc.y); // +Y
        case 3: return vec3( ndc.x, -1.0,  ndc.y); // -Y
        case 4: return vec3( ndc.x, -ndc.y,  1.0); // +Z
        case 5: return vec3(-ndc.x, -ndc.y, -1.0); // -Z
        default: return vec3(0,0,0);
    }
}

void main() {
    v_Direction = normalize(GetDirection(pc.u_FaceIndex, a_TexCoord));
    gl_Position = vec4(a_Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 v_Direction;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform sampler2D u_EquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{       
    vec2 uv = SampleSphericalMap(normalize(v_Direction)); // make sure to normalize v_Direction
    vec3 color = texture(u_EquirectangularMap, uv).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    o_Color = vec4(color, 1.0);
}

#endif
