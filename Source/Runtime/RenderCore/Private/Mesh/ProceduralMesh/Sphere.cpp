#include "Sphere.h"

DEFINE_LOG_CATEGORY(LogSphere);

SIMPLE_MESH_PROPERTY(FSphereParams, Radius, float, Radius)
SIMPLE_MESH_PROPERTY(FSphereParams, WidthSegments, uint32_t, WidthSegments)
SIMPLE_MESH_PROPERTY(FSphereParams, HeightSegments, uint32_t, HeightSegments)

// Properties with defaul value
DEFINE_MESH_PROPERTY(FSphereParams, PhiStart, float, PhiStart,
                     params.PhiLength  = ChozoUtils::Math::PI * 2;
                     params.ThetaStart = 0.0f; params.ThetaLength = ChozoUtils::Math::PI;)

DEFINE_MESH_PROPERTY(FSphereParams, PhiLength, float, PhiLength, params.PhiStart = 0.0f;
                     params.ThetaStart = 0.0f; params.ThetaLength = ChozoUtils::Math::PI;)

DEFINE_MESH_PROPERTY(FSphereParams, ThetaStart, float, ThetaStart, params.PhiStart = 0.0f;
                     params.PhiLength   = ChozoUtils::Math::PI * 2;
                     params.ThetaLength = ChozoUtils::Math::PI;)

DEFINE_MESH_PROPERTY(FSphereParams, ThetaLength, float, ThetaLength, params.PhiStart = 0.0f;
                     params.PhiLength = ChozoUtils::Math::PI * 2; params.ThetaStart = 0.0f;)

FSphere::FSphere(float radius, uint32 widthSegments, uint32 heightSegments, float phiStart,
                 float phiLength, float thetaStart, float thetaLength) {
    SetRadius(radius);
    SetWidthSegments(widthSegments);
    SetHeightSegments(heightSegments);
    SetPhiStart(phiStart);
    SetPhiLength(phiLength);
    SetThetaStart(thetaStart);
    SetThetaLength(thetaLength);

    GenerateBuffer();
}

FSphere::FSphere(const FSphereParams& params) : m_Params(params) { GenerateBuffer(); }

// FSphere::FSphere(const FSphereParams& params, const FMeshBuffer* data)
//     : FProceduralMesh(data), m_Params(params) {}

FMeshBuffer* FSphere::GenerateBuffer() {
    m_Buffer.Clear();

    unsigned int width_segments  = std::max(3u, m_Params.WidthSegments);
    unsigned int height_segments = std::max(2u, m_Params.HeightSegments);

    float thetaEnd = std::min(m_Params.ThetaStart + m_Params.ThetaLength, ChozoUtils::Math::PI);

    unsigned int index = 0;
    std::vector<std::vector<float>> grid;

    for (unsigned int iy = 0; iy <= height_segments; iy++) {
        std::vector<float> verticesRow;
        float v       = static_cast<float>(iy) / height_segments;
        float uOffset = 0;
        if (iy == 0 && m_Params.ThetaStart == 0) {
            uOffset = 0.5 / width_segments;
        } else if (iy == height_segments && thetaEnd == ChozoUtils::Math::PI) {
            uOffset = -0.5 / width_segments;
        }

        for (unsigned int ix = 0; ix <= width_segments; ix++) {
            float u = static_cast<float>(ix) / width_segments;

            float xPos = -m_Params.Radius * std::cos(m_Params.PhiStart + u * m_Params.PhiLength) *
                         std::sin(m_Params.ThetaStart + v * m_Params.ThetaLength);
            float yPos = m_Params.Radius * std::cos(m_Params.ThetaStart + v * m_Params.ThetaLength);
            float zPos = m_Params.Radius * std::sin(m_Params.PhiStart + u * m_Params.PhiLength) *
                         std::sin(m_Params.ThetaStart + v * m_Params.ThetaLength);

            FVertex vertice;

            // vertice.Position =
            //     (m_LocalTransform * FVector4(FVector3(xPos, yPos, zPos), 1.0f)).ToVector3();

            vertice.Position = FVector3(xPos, yPos, zPos);
            vertice.Normal   = FVector3(xPos, yPos, zPos).Normalized();

            // CZ_LOG(LogSphere, Trace, "GenerateBuffer: Position {}", vertice.Position.ToString());
            // CZ_LOG(LogSphere, Trace, "GenerateBuffer: Normal {}", vertice.Normal.ToString());

            vertice.UV.x = u + uOffset;
            vertice.UV.y = 1 - v;

            verticesRow.push_back(index++);
            m_Buffer.Vertices.push_back(vertice);
        }

        grid.push_back(verticesRow);
    }

    for (uint32 iy = 0; iy < height_segments; iy++) {
        for (uint32 ix = 0; ix < width_segments; ix++) {
            uint32 a = grid[iy][ix + 1];
            uint32 b = grid[iy][ix];
            uint32 c = grid[iy + 1][ix];
            uint32 d = grid[iy + 1][ix + 1];

            if (iy != 0 || m_Params.ThetaStart > 0) {
                m_Buffer.Indices.insert(m_Buffer.Indices.end(), { a, b, d });
            }

            if (iy != height_segments - 1 || thetaEnd < ChozoUtils::Math::PI) {
                m_Buffer.Indices.insert(m_Buffer.Indices.end(), { b, c, d });
            }
        }
    }

    return &m_Buffer;
}
