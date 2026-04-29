vec3 PackNormalToRGB(const in vec3 normal)
{
    return normalize(normal) * 0.5 + 0.5;
}

vec3 UnpackRGBToNormal(const in vec3 rgb)
{
    return 2.0 * rgb.xyz - 1.0;
}

float LinearizeNDCDepthToViewZ(const in float ndcDepth, const in float near, const in float far)
{
    float z = ndcDepth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

// NOTE: https://twitter.com/gonnavis/status/1377183786949959682
float ViewZToNDC(const in float viewZ, const in float near, const in float far) {
    // -near maps to 0; -far maps to 1
    return (( near + viewZ ) * far ) / ( ( far - near ) * viewZ);
}

float NDCToViewZ(const in float depth, const in float near, const in float far) {
    // maps z_{NDC}(perspective depth) in [ 0, 1 ] to z_{view}
    return (near * far ) / ( ( far - near ) * depth - far);
}
