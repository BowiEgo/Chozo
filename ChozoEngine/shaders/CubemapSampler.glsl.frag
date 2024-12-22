#version 450

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_WorldPosition;

layout(binding = 0) uniform sampler2D u_EquirectangularMap;

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
    vec2 uv = SampleSphericalMap(normalize(v_WorldPosition)); // make sure to normalize v_WorldPosition
    vec3 color = texture(u_EquirectangularMap, uv).rgb;

    // color = color / (color + vec3(1.0));
    // color = pow(color, vec3(1.0/2.2));

    o_Color = vec4(color, 1.0);
}