#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>
#include <Runtime/RenderCore/ProceduralMesh/SphereParamsObj.hpp>

namespace CZ {

/// UV sphere mirroring the dev-vulkan reference implementation: same triangle winding as the cube,
/// pole UV offsets of half a texel and skipped degenerate triangles at the poles.
class SphereGenerator : public MeshGenerator {
public:
    MeshBuffer* GenerateBuffer(MeshParams params, MeshBuffer* buffer) override {
        auto sphere = params.As<SphereParamsObj>();
        if (!sphere || !buffer) return nullptr;

        constexpr float kPi = 3.14159265358979323846f;

        const uint32_t widthSegments  = std::max(3u, sphere->WidthSegments);
        const uint32_t heightSegments = std::max(2u, sphere->HeightSegments);
        const float radius            = sphere->Radius;
        const float thetaEnd          = std::min(sphere->ThetaStart + sphere->ThetaLength, kPi);

        uint32_t index = 0;
        std::vector<std::vector<uint32_t>> grid;

        for (uint32_t iy = 0; iy <= heightSegments; iy++) {
            std::vector<uint32_t> verticesRow;
            const float v = static_cast<float>(iy) / static_cast<float>(heightSegments);

            float uOffset = 0.0f;
            if (iy == 0 && sphere->ThetaStart == 0.0f) {
                uOffset = 0.5f / static_cast<float>(widthSegments);
            } else if (iy == heightSegments && thetaEnd == kPi) {
                uOffset = -0.5f / static_cast<float>(widthSegments);
            }

            for (uint32_t ix = 0; ix <= widthSegments; ix++) {
                const float u     = static_cast<float>(ix) / static_cast<float>(widthSegments);
                const float phi   = sphere->PhiStart + u * sphere->PhiLength;
                const float theta = sphere->ThetaStart + v * sphere->ThetaLength;

                const float xPos = -radius * std::cos(phi) * std::sin(theta);
                const float yPos = radius * std::cos(theta);
                const float zPos = radius * std::sin(phi) * std::sin(theta);

                const Vector3 position(xPos, yPos, zPos);
                buffer->Vertices.emplace_back(position, position.Normalized(),
                                              Vector2(u + uOffset, 1.0f - v));
                verticesRow.push_back(index++);
            }

            grid.push_back(verticesRow);
        }

        for (uint32_t iy = 0; iy < heightSegments; iy++) {
            for (uint32_t ix = 0; ix < widthSegments; ix++) {
                const uint32_t a = grid[iy][ix + 1];
                const uint32_t b = grid[iy][ix];
                const uint32_t c = grid[iy + 1][ix];
                const uint32_t d = grid[iy + 1][ix + 1];

                if (iy != 0 || sphere->ThetaStart > 0.0f) {
                    buffer->Indices.insert(buffer->Indices.end(), { a, b, d });
                }
                if (iy != heightSegments - 1 || thetaEnd < kPi) {
                    buffer->Indices.insert(buffer->Indices.end(), { b, c, d });
                }
            }
        }

        return buffer;
    }
};

} // namespace CZ
