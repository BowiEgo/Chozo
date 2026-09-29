#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>
#include <Runtime/RenderCore/ProceduralMesh/TorusParamsObj.hpp>

namespace CZ {

/// Torus around the Y axis, built on the same ring grid (and winding) as the sphere side wall.
class TorusGenerator : public MeshGenerator {
public:
    MeshBuffer* GenerateBuffer(MeshParams params, MeshBuffer* buffer) override {
        auto torus = params.As<TorusParamsObj>();
        if (!torus || !buffer) return nullptr;

        constexpr float kPi = 3.14159265358979323846f;

        const uint32_t radialSegments  = std::max(3u, torus->RadialSegments);
        const uint32_t tubularSegments = std::max(3u, torus->TubularSegments);
        const float radius             = torus->Radius;
        const float tube               = torus->Tube;

        for (uint32_t iy = 0; iy <= tubularSegments; ++iy) {
            const float v    = static_cast<float>(iy) / static_cast<float>(tubularSegments);
            const float phi  = v * 2.0f * kPi;
            const float ring = radius + tube * std::cos(phi);
            for (uint32_t ix = 0; ix <= radialSegments; ++ix) {
                const float u     = static_cast<float>(ix) / static_cast<float>(radialSegments);
                const float theta = u * 2.0f * kPi;
                const Vector3 normal(std::cos(phi) * std::cos(theta), std::sin(phi),
                                     std::cos(phi) * std::sin(theta));
                buffer->Vertices.emplace_back(
                    Vector3(ring * std::cos(theta), tube * std::sin(phi), ring * std::sin(theta)),
                    normal, Vector2(u, v));
            }
        }

        for (uint32_t iy = 0; iy < tubularSegments; ++iy) {
            for (uint32_t ix = 0; ix < radialSegments; ++ix) {
                const uint32_t a = iy * (radialSegments + 1) + ix + 1;
                const uint32_t b = iy * (radialSegments + 1) + ix;
                const uint32_t c = (iy + 1) * (radialSegments + 1) + ix;
                const uint32_t d = (iy + 1) * (radialSegments + 1) + ix + 1;
                buffer->Indices.insert(buffer->Indices.end(), { a, b, d });
                buffer->Indices.insert(buffer->Indices.end(), { b, c, d });
            }
        }

        return buffer;
    }
};

} // namespace CZ
