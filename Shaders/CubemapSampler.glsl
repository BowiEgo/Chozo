#version 460

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 a_Position; // 顶点位置（范围 -1 到 1）

layout(location = 0) out vec3 v_WorldPosition;

// uniform mat4 u_Model; // 模型矩阵（恒等即可）

void main() {
    mat4 u_Model = mat4(1.0);
    v_WorldPosition = a_Position; // 世界方向 = 立方体顶点位置（假设模型矩阵为单位，且相机在原点）
    gl_Position = u_Model * vec4(a_Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

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

#endif
