#include "Sphere.h"

FSphere::FSphere(float radius, uint32 widthSegments, uint32 heightSegments, float phiStart,
                 float phiLength, float thetaStart, float thetaLength)
    : m_Radius(radius), m_WidthSegments(widthSegments), m_HeightSegments(heightSegments),
      m_PhiStart(phiStart), m_PhiLength(phiLength), m_ThetaStart(thetaStart),
      m_ThetaLength(thetaLength) {
    m_Buffer = Generate();
}

FMeshBuffer* FSphere::Generate() {
    FMeshBuffer* buffer = new FMeshBuffer();

    unsigned int width_segments = std::max(3u, m_WidthSegments);
    unsigned int height_segments = std::max(2u, m_HeightSegments);

    float thetaEnd = std::min(m_ThetaStart + m_ThetaLength, ChozoUtils::Math::PI);

    unsigned int index = 0;
    std::vector<std::vector<float>> grid;

    for (unsigned int iy = 0; iy <= height_segments; iy++) {
        std::vector<float> verticesRow;
        float v = static_cast<float>(iy) / height_segments;
        float uOffset = 0;
        if (iy == 0 && m_ThetaStart == 0) {
            uOffset = 0.5 / width_segments;
        } else if (iy == height_segments && thetaEnd == ChozoUtils::Math::PI) {
            uOffset = -0.5 / width_segments;
        }

        for (unsigned int ix = 0; ix <= width_segments; ix++) {
            float u = static_cast<float>(ix) / width_segments;

            float xPos = -m_Radius * std::cos(m_PhiStart + u * m_PhiLength) *
                         std::sin(m_ThetaStart + v * m_ThetaLength);
            float yPos = m_Radius * std::cos(m_ThetaStart + v * m_ThetaLength);
            float zPos = m_Radius * std::sin(m_PhiStart + u * m_PhiLength) *
                         std::sin(m_ThetaStart + v * m_ThetaLength);

            FVertex vertice;

            vertice.Position.x = xPos;
            vertice.Position.y = yPos;
            vertice.Position.z = zPos;
            vertice.Position = (m_LocalTransform * FVector4(vertice.Position, 1.0f)).ToVector3();

            vertice.Normal.x = xPos;
            vertice.Normal.y = yPos;
            vertice.Normal.z = zPos;

            vertice.TexCoord.x = u + uOffset;
            vertice.TexCoord.y = 1 - v;

            verticesRow.push_back(index++);
            buffer->Vertexs.push_back(vertice);
        }

        grid.push_back(verticesRow);
    }

    for (uint32 iy = 0; iy < height_segments; iy++) {
        for (uint32 ix = 0; ix < width_segments; ix++) {
            uint32 a = grid[iy][ix + 1];
            uint32 b = grid[iy][ix];
            uint32 c = grid[iy + 1][ix];
            uint32 d = grid[iy + 1][ix + 1];

            if (iy != 0 || m_ThetaStart > 0) {
                buffer->Indices.insert(buffer->Indices.end(), { a, b, d });
                buffer->IndexCount += 3;
            }

            if (iy != height_segments - 1 || thetaEnd < ChozoUtils::Math::PI) {
                buffer->Indices.insert(buffer->Indices.end(), { b, c, d });
                buffer->IndexCount += 3;
            }
        }
    }

    return buffer;
}

void FSphere::Backup() {
    m_OldRadius = m_Radius;
    m_OldPhiStart = m_PhiStart;
    m_OldPhiLength = m_PhiLength;
    m_OldThetaStart = m_ThetaStart;
    m_OldThetaLength = m_ThetaLength;
    m_OldWidthSegments = m_WidthSegments;
    m_OldHeightSegments = m_HeightSegments;
}

void FSphere::Backtrace() {
    m_Radius = m_OldRadius;
    m_PhiStart = m_OldPhiStart;
    m_PhiLength = m_OldPhiLength;
    m_ThetaStart = m_OldThetaStart;
    m_ThetaLength = m_OldThetaLength;
    m_WidthSegments = m_OldWidthSegments;
    m_HeightSegments = m_OldHeightSegments;
}
