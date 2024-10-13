#include "SphereGeometry.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

namespace Chozo
{
    SphereGeometry::SphereGeometry(float radius, uint32_t widthSegments, uint32_t heightSegments,
        float phiStart, float phiLength, float thetaStart, float thetaLength)
        : m_Radius(radius), m_WidthSegments(widthSegments), m_HeightSegments(heightSegments),
          m_PhiStart(phiStart), m_PhiLength(phiLength), m_ThetaStart(thetaStart), m_ThetaLength(thetaLength)
    {
        CallGenerate();
    }

    MeshBuffer* SphereGeometry::Generate()
    {
        MeshBuffer* buffer = new MeshBuffer();

        unsigned int width_segments = std::max(3u, m_WidthSegments);
        unsigned int height_segments = std::max(2u, m_HeightSegments);

        float thetaEnd = std::min(m_ThetaStart + m_ThetaLength, Math::PI);

        unsigned int index = 0;
        std::vector<std::vector<float>> grid;

        for (unsigned int iy = 0; iy <= height_segments; iy++) {
            std::vector<float> verticesRow;
            float v = static_cast<float>(iy) / height_segments;
            float uOffset = 0;
            if (iy == 0 && m_ThetaStart == 0) {
                uOffset = 0.5 / width_segments;
            } else if (iy == height_segments && thetaEnd == Math::PI) {
                uOffset = -0.5 / width_segments;
            }

            for (unsigned int ix = 0; ix <= width_segments; ix++) {
                float u = static_cast<float>(ix) / width_segments;

                float xPos = - m_Radius * std::cos(m_PhiStart   + u * m_PhiLength) * std::sin(m_ThetaStart + v * m_ThetaLength);
                float yPos =   m_Radius * std::cos(m_ThetaStart + v * m_ThetaLength);
                float zPos =   m_Radius * std::sin(m_PhiStart   + u * m_PhiLength) * std::sin(m_ThetaStart + v * m_ThetaLength);

                Vertex vertice;

                vertice.Position.x = xPos;
                vertice.Position.y = yPos;
                vertice.Position.z = zPos;
                vertice.Position = m_LocalTransform * glm::vec4(vertice.Position, 1.0f);

                vertice.Normal.x = xPos;
                vertice.Normal.y = yPos;
                vertice.Normal.z = zPos;

                vertice.TexCoord.x = u + uOffset;
                vertice.TexCoord.y = 1 - v;

                vertice.EntityID = m_EntityID;

                verticesRow.push_back(index++);
                buffer->Vertexs.push_back(vertice);
            }

            grid.push_back(verticesRow);
        }

        for (uint32_t iy = 0; iy < height_segments; iy++ ) {
            for (uint32_t ix = 0; ix < width_segments; ix++) {
                uint32_t a = grid[iy][ix + 1];
                uint32_t b = grid[iy][ix];
                uint32_t c = grid[iy + 1][ix];
                uint32_t d = grid[iy + 1][ix + 1];

                if (iy != 0 || m_ThetaStart > 0)
                {
                    buffer->Indexs.insert(buffer->Indexs.end(), {a, b, d});
                    buffer->IndicesCount += 3;
                    buffer->IndexCount++;
                }

                if (iy != height_segments - 1 || thetaEnd < Math::PI)
                {
                    buffer->Indexs.insert(buffer->Indexs.end(), {b, c, d});
                    buffer->IndicesCount += 3;
                    buffer->IndexCount++;
                }
            }
        }

        return buffer;
    }

    void SphereGeometry::Backup()
    {
        m_OldRadius = m_Radius;
        m_OldPhiStart = m_PhiStart;
        m_OldPhiLength = m_PhiLength;
        m_OldThetaStart = m_ThetaStart;
        m_OldThetaLength = m_ThetaLength;
        m_OldWidthSegments = m_WidthSegments;
        m_OldHeightSegments = m_HeightSegments;
    }

    void SphereGeometry::Backtrace()
    {
        m_Radius = m_OldRadius;
        m_PhiStart = m_OldPhiStart;
        m_PhiLength = m_OldPhiLength;
        m_ThetaStart = m_OldThetaStart;
        m_ThetaLength = m_OldThetaLength;
        m_WidthSegments = m_OldWidthSegments;
        m_HeightSegments = m_OldHeightSegments;
    }
}
