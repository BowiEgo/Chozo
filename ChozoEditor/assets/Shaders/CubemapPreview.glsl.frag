#version 450
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPosition;

layout(binding = 0) uniform samplerCube u_Texture;

void main()
{
    vec2 uv = v_TexCoord;
    vec3 dir = vec3(0.0);
    vec3 color = vec3(0.0);

    if (uv.x < 0.25) {
        if (uv.y > 0.33 && uv.y < 0.66) {
            // left (-X)
            dir = vec3(-1.0, (uv.y - 0.33) * 3.0 - 0.5, (1.0 - uv.x * 4.0) - 0.5);
        }
    } else if (uv.x < 0.5) {
        if (uv.y > 0.66) {
            // Top (+Y)
            dir = vec3((uv.x - 0.25) * 4.0 - 0.5, 1.0, (uv.y - 0.66) * 3.0 - 0.5);
        } else if (uv.y > 0.33) {
            // Front (+Z)
            dir = vec3((uv.x - 0.25) * 4.0 - 0.5, (uv.y - 0.33) * 3.0 - 0.5, -1.0);
        } else {
            // Bottom (-Y)
            color = vec3(0.0, 0.0, 1.0);
            dir = vec3((uv.x - 0.25) * 4.0 - 0.5, -1.0, uv.y * 3.0 - 0.5);
        }
    } else if (uv.x < 0.75) {
        if (uv.y > 0.33 && uv.y < 0.66) {
            // Right (+X)
            dir = vec3(1.0, (uv.y - 0.33) * 3.0 - 0.5, (1.0 - (uv.x - 0.5) * 4.0) - 0.5);
        }
    } else {
        if (uv.y > 0.33 && uv.y < 0.66) {
            // Back (-Z)
            dir = vec3((1.0 - (uv.x - 0.75) * 4.0) - 0.5, (uv.y - 0.33) * 3.0 - 0.5, 1.0);
        }
    }

    if (dir != vec3(0.0))
    {
        dir = normalize(dir);
        color = texture(u_Texture, dir).rgb;
    } else {
        discard;
    }
    
    o_Color = vec4(color, 1.0);
}