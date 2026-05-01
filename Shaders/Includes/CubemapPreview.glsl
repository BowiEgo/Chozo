vec4 CubemapPreview(const in samplerCube u_textureCube, const in vec2 uv)
{
    vec3 color = vec3(0.0);
    vec3 dir = vec3(0.0);
    float alpha = 1.0;

    float localY = (0.5 - (uv.y - 0.333) * 3.0) * 2.0;

    if (uv.y > 0.333 && uv.y < 0.666) {
        if (uv.x < 0.25) { 
            // 1. Left (-X)
            float localZ = (uv.x / 0.25) * 2.0 - 1.0; 
            dir = vec3(-1.0, localY, -localZ);
        } else if (uv.x < 0.5) { 
            // 2. Front (-Z)
            float localX = ((uv.x - 0.25) / 0.25) * 2.0 - 1.0;
            dir = vec3(localX, localY, -1.0);
        } else if (uv.x < 0.75) { 
            // 3. Right (+X)
            float localZ = ((uv.x - 0.5) / 0.25) * 2.0 - 1.0;
            dir = vec3(1.0, localY, localZ);
        } else { 
            // 4. Back (+Z)
            float localX = ((uv.x - 0.75) / 0.25) * 2.0 - 1.0;
            dir = vec3(-localX, localY, 1.0);
        }
    } else if (uv.x > 0.25 && uv.x < 0.5) {
        float localX = ((uv.x - 0.25) / 0.25) * 2.0 - 1.0;
        
        if (uv.y <= 0.333) { 
            // Top (+Y)
            float depth = (1.0 - (uv.y / 0.333)) * 2.0 - 1.0;
            dir = vec3(localX, 1.0, depth);
        } else if (uv.y >= 0.666) { 
            // Bottom (-Y)
            float depth = ((uv.y - 0.666) / 0.333) * 2.0 - 1.0;
            dir = vec3(localX, -1.0, depth);
        }
    } else {
        alpha = 0.0;
    }

    if (alpha > 0.0) {
        color = texture(u_textureCube, normalize(dir)).rgb;
    }

    return vec4(color, alpha);
}