#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include <Runtime/RenderCore/ProceduralMesh/PlaneParamsObj.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

/// Subdivided XY plane facing +Z, laid out like the dev-vulkan Quad reference (same vertex order,
/// UVs and triangle winding) and generalised to a grid.
class PlaneGenerator : public MeshGenerator {
public:
    MeshBuffer* GenerateBuffer(MeshParams params, MeshBuffer* buffer) override {
        auto plane = params.As<PlaneParamsObj>();
        if (!plane || !buffer) return nullptr;

        const uint32_t widthSegments  = std::max(1u, plane->WidthSegments);
        const uint32_t heightSegments = std::max(1u, plane->HeightSegments);

        for (uint32_t iy = 0; iy <= heightSegments; ++iy) {
            const float v = static_cast<float>(iy) / static_cast<float>(heightSegments);
            for (uint32_t ix = 0; ix <= widthSegments; ++ix) {
                const float u = static_cast<float>(ix) / static_cast<float>(widthSegments);
                buffer->Vertices.emplace_back(
                    Vector3((u - 0.5f) * plane->Width, (v - 0.5f) * plane->Height, 0.0f),
                    Vector3(0.0f, 0.0f, 1.0f), Vector2(u, 1.0f - v));
            }
        }

        for (uint32_t iy = 0; iy < heightSegments; ++iy) {
            for (uint32_t ix = 0; ix < widthSegments; ++ix) {
                const uint32_t a = iy * (widthSegments + 1) + ix;
                const uint32_t b = a + 1;
                const uint32_t c = a + widthSegments + 2;
                const uint32_t d = a + widthSegments + 1;
                buffer->Indices.insert(buffer->Indices.end(), { a, b, c });
                buffer->Indices.insert(buffer->Indices.end(), { c, d, a });
            }
        }

        return buffer;
    }
};

} // namespace CZ
