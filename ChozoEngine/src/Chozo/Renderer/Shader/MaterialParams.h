#pragma once

#include <glm/glm.hpp>

namespace Chozo {

    enum MaterialFlags {
        EnableBaseColorMap  = 1 << 0,
        EnableMetallicMap   = 1 << 1,
        EnableRoughnessMap  = 1 << 2,
        EnableNormalMap     = 1 << 3,
        EnableEmissiveMap   = 1 << 4,
        EnableOcclusionMap  = 1 << 5,
    };

    struct MaterialParams
    {
        glm::vec4 BaseColor = { 1.0f, 1.0f, 1.0f, 1.0f };  // Base color
        glm::vec4 Emissive = { 0.0f, 0.0f, 0.0f, 0.0f };   // Emissive.rgb and EmissiveIntensity
        glm::vec3 MiscParams = { 1.0f, 1.0f, 1.0f };       // Metallic, Roughness, OcclusionIntensity
        uint32_t Flags = 0;                                // Bitmask for all Enable* flags
    };
}