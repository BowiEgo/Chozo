#include "BoxGeometry.h"

#include <glm/glm.hpp>

namespace Chozo
{
    static void SetAxisValue(glm::vec3& target, const std::string& axis, float value) {
        if (axis == "x") {
            target.x = value;
        } else if (axis == "y") {
            target.y = value;
        } else if (axis == "z") {
            target.z = value;
        }
    }

    BoxGeometry::BoxGeometry(const float &width, const float &height, const float &depth, const uint32_t &widthSegments, const uint32_t &heightSegments, const uint32_t &depthSegments)
        : m_Width(width), m_Height(height), m_Depth(depth),
          m_WidthSegments(widthSegments), m_HeightSegments(heightSegments), m_DepthSegments(depthSegments)
    {
        // build each side of the box geometry
        BuildPlane("z", "y", "x", -1, -1, depth, height,  width,  depthSegments, heightSegments); // px
		BuildPlane("z", "y", "x",  1, -1, depth, height, -width,  depthSegments, heightSegments); // nx
		BuildPlane("x", "z", "y",  1,  1, width, depth,   height, widthSegments, depthSegments); // py
		BuildPlane("x", "z", "y",  1, -1, width, depth,  -height, widthSegments, depthSegments); // ny
		BuildPlane("x", "y", "z",  1, -1, width, height,  depth,  widthSegments, heightSegments); // pz
		BuildPlane("x", "y", "z", -1, -1, width, height, -depth,  widthSegments, heightSegments); // nz
    }

    void BoxGeometry::BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width, float height, float depth, uint32_t gridX, uint32_t gridY)
    {
        const float segmentWidth = width / gridX;
        const float segmentHeight = height / gridY;

        const float widthHalf = width / 2;
        const float heightHalf = height / 2;
        const float depthHalf = depth / 2;

        const uint32_t gridX1 = gridX + 1;
        const uint32_t gridY1 = gridY + 1;

        int vertexCounter = 0;

        glm::vec3 vector;

        // Vertices
        for (int iy = 0; iy < gridY1; iy++ )
        {
            const float y = iy * segmentHeight - heightHalf;

            for (int ix = 0; ix < gridX1; ix++ ) {

                const float x = ix * segmentWidth - widthHalf;

                Vertex vertice;

                SetAxisValue(vertice.Position, u, x * uDir);
                SetAxisValue(vertice.Position, v, y * vDir);
                SetAxisValue(vertice.Position, w, depthHalf);

                SetAxisValue(vertice.Normal, u, 0);
                SetAxisValue(vertice.Normal, v, 0);
                SetAxisValue(vertice.Normal, w, depth > 0 ? 1 : -1);

                vertice.TexCoord.x = ix / gridX;
                vertice.TexCoord.y = 1 - ( iy / gridY );

                m_Vertices.push_back(vertice);
                vertexCounter += 1;
            }
        }

        // Indices
        // 1. you need three indices to draw a single face
        // 2. a single segment consists of two faces
        // 3. so we need to generate six (2*3) indices per segment
        for (uint32_t iy = 0; iy < gridY; iy ++ )
        {
            for (uint32_t ix = 0; ix < gridX; ix ++ )
            {
                const uint32_t a = m_NumberOfVertices + ix + gridX1 * iy;
                const uint32_t b = m_NumberOfVertices + ix + gridX1 * ( iy + 1 );
                const uint32_t c = m_NumberOfVertices + ( ix + 1 ) + gridX1 * ( iy + 1 );
                const uint32_t d = m_NumberOfVertices + ( ix + 1 ) + gridX1 * iy;

                m_Indices.insert(m_Indices.end(), {a, b, d});
                m_Indices.insert(m_Indices.end(), {b, c, d});

                m_IndexCount += 6;
                m_TriangleCount += 2;
            }
        }

        m_NumberOfVertices += vertexCounter;
    }
}
