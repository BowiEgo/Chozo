#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include <Runtime/RenderCore/ProceduralMesh/CylinderParamsObj.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

/// Y-axis cylinder: a radial side wall plus two disks. The caps are emitted with both windings so a
/// wrong cap orientation cannot hide them (depth testing makes the redundant triangles invisible).
class CylinderGenerator : public MeshGenerator {
public:
    MeshBuffer* GenerateBuffer(MeshParams params, MeshBuffer* buffer) override {
        auto cylinder = params.As<CylinderParamsObj>();
        if (!cylinder || !buffer) return nullptr;

        constexpr float kPi = 3.14159265358979323846f;

        const uint32_t radialSegments = std::max(3u, cylinder->RadialSegments);
        const uint32_t heightSegments = std::max(1u, cylinder->HeightSegments);
        const float radius            = cylinder->Radius;
        const float halfHeight        = cylinder->Height * 0.5f;

        for (uint32_t iy = 0; iy <= heightSegments; ++iy) {
            const float v = static_cast<float>(iy) / static_cast<float>(heightSegments);
            const float y = (v - 0.5f) * cylinder->Height;
            for (uint32_t ix = 0; ix <= radialSegments; ++ix) {
                const float u     = static_cast<float>(ix) / static_cast<float>(radialSegments);
                const float theta = u * 2.0f * kPi;
                const Vector3 normal(std::cos(theta), 0.0f, std::sin(theta));
                buffer->Vertices.emplace_back(Vector3(normal.x * radius, y, normal.z * radius),
                                              normal, Vector2(u, v));
            }
        }

        for (uint32_t iy = 0; iy < heightSegments; ++iy) {
            for (uint32_t ix = 0; ix < radialSegments; ++ix) {
                const uint32_t a = iy * (radialSegments + 1) + ix + 1;
                const uint32_t b = iy * (radialSegments + 1) + ix;
                const uint32_t c = (iy + 1) * (radialSegments + 1) + ix;
                const uint32_t d = (iy + 1) * (radialSegments + 1) + ix + 1;
                buffer->Indices.insert(buffer->Indices.end(), { a, b, d });
                buffer->Indices.insert(buffer->Indices.end(), { b, c, d });
            }
        }

        for (uint32_t cap = 0; cap < 2; ++cap) {
            const float sign      = cap == 0 ? 1.0f : -1.0f;
            const uint32_t center = static_cast<uint32_t>(buffer->Vertices.size());
            buffer->Vertices.emplace_back(Vector3(0.0f, sign * halfHeight, 0.0f),
                                          Vector3(0.0f, sign, 0.0f), Vector2(0.5f, 0.5f));

            const uint32_t first = static_cast<uint32_t>(buffer->Vertices.size());
            for (uint32_t ix = 0; ix <= radialSegments; ++ix) {
                const float theta =
                    static_cast<float>(ix) / static_cast<float>(radialSegments) * 2.0f * kPi;
                const Vector3 normal(std::cos(theta), 0.0f, std::sin(theta));
                buffer->Vertices.emplace_back(
                    Vector3(normal.x * radius, sign * halfHeight, normal.z * radius),
                    Vector3(0.0f, sign, 0.0f),
                    Vector2(0.5f + normal.x * 0.5f, 0.5f + normal.z * 0.5f));
            }

            for (uint32_t ix = 0; ix < radialSegments; ++ix) {
                const uint32_t p0 = first + ix;
                const uint32_t p1 = first + ix + 1;
                buffer->Indices.insert(buffer->Indices.end(), { center, p0, p1 });
                buffer->Indices.insert(buffer->Indices.end(), { center, p1, p0 });
            }
        }

        return buffer;
    }
};

} // namespace CZ
