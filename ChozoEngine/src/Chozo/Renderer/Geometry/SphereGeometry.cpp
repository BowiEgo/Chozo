#include "SphereGeometry.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

namespace Chozo
{

    SphereGeometry::SphereGeometry(float radius, unsigned int widthSegments, unsigned int heightSegments, float phiStart, float phiLength, float thetaStart, float thetaLength)
    {
        unsigned int width_segments = std::max(3u, widthSegments);
        unsigned int height_segments = std::max(2u, heightSegments);

        float thetaEnd = std::min(thetaStart + thetaLength, Math::PI);

        unsigned int index = 0;
        std::vector<std::vector<float>> grid;

        for (unsigned int iy = 0; iy <= height_segments; iy++) {
            std::vector<float> verticesRow;
            float v = static_cast<float>(iy) / height_segments;
            float uOffset = 0;
            if (iy == 0 && thetaStart == 0) {
                uOffset = 0.5 / width_segments;
            } else if (iy == height_segments && thetaEnd == Math::PI) {
                uOffset = -0.5 / width_segments;
            }

            for (unsigned int ix = 0; ix <= width_segments; ix++) {
                float u = static_cast<float>(ix) / width_segments;

                float xPos = - radius * std::cos(phiStart   + u * phiLength) * std::sin(thetaStart + v * thetaLength);
                float yPos =   radius * std::cos(thetaStart + v * thetaLength);
                float zPos =   radius * std::sin(phiStart   + u * phiLength) * std::sin(thetaStart + v * thetaLength);

                Vertex vertice;

                vertice.Position.x = xPos;
                vertice.Position.y = yPos;
                vertice.Position.z = zPos;

                vertice.Normal.x = xPos;
                vertice.Normal.y = yPos;
                vertice.Normal.z = zPos;

                vertice.TexCoord.x = u + uOffset;
                vertice.TexCoord.x = 1 - v;

                m_Vertices.push_back(vertice);
                verticesRow.push_back(index++);
            }

            grid.push_back(verticesRow);
        }

        for (uint32_t iy = 0; iy < height_segments; iy++ ) {
            for (uint32_t ix = 0; ix < width_segments; ix++) {
                uint32_t a = grid[iy][ix + 1];
                uint32_t b = grid[iy][ix];
                uint32_t c = grid[iy + 1][ix];
                uint32_t d = grid[iy + 1][ix + 1];

                if (iy != 0 || thetaStart > 0)
                {
                    m_Indices.insert(m_Indices.end(), {a, b, d});
                    m_IndexCount += 3;
                    m_TriangleCount++;
                }

                if (iy != height_segments - 1 || thetaEnd < Math::PI)
                {
                    m_Indices.insert(m_Indices.end(), {b, c, d});
                    m_IndexCount += 3;
                    m_TriangleCount++;
                }
            }
        }

        CZ_CORE_INFO(m_TriangleCount);
    }
}
