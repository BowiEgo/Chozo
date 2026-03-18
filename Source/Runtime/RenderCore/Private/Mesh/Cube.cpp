#include "Cube.h"

static void SetAxisValue(FVector3& target, const std::string& axis, float value) {
    if (axis == "x") {
        target.x = value;
    } else if (axis == "y") {
        target.y = value;
    } else if (axis == "z") {
        target.z = value;
    }
}

FCube::FCube(const float& width, const float& height, const float& depth,
             const uint32& widthSegments, const uint32& heightSegments, const uint32& depthSegments)
    : m_Width(width), m_Height(height), m_Depth(depth), m_WidthSegments(widthSegments),
      m_HeightSegments(heightSegments), m_DepthSegments(depthSegments) {
    m_Buffer = Generate();
}

FMeshBuffer* FCube::Generate() {
    m_NumberOfVertices = 0;

    FMeshBuffer* buffer = new FMeshBuffer();
    // build each side of the box geometry
    BuildPlane("z", "y", "x", -1, -1, m_Depth, m_Height, m_Width, m_DepthSegments, m_HeightSegments,
               buffer); // px
    BuildPlane("z", "y", "x", 1, -1, m_Depth, m_Height, -m_Width, m_DepthSegments, m_HeightSegments,
               buffer); // nx
    BuildPlane("x", "z", "y", 1, 1, m_Width, m_Depth, m_Height, m_WidthSegments, m_DepthSegments,
               buffer); // py
    BuildPlane("x", "z", "y", 1, -1, m_Width, m_Depth, -m_Height, m_WidthSegments, m_DepthSegments,
               buffer); // ny
    BuildPlane("x", "y", "z", 1, -1, m_Width, m_Height, m_Depth, m_WidthSegments, m_HeightSegments,
               buffer); // pz
    BuildPlane("x", "y", "z", -1, -1, m_Width, m_Height, -m_Depth, m_WidthSegments,
               m_HeightSegments, buffer); // nz

    return buffer;
}

void FCube::Backup() {
    m_OldWidth = m_Width;
    m_OldHeight = m_Height;
    m_OldDepth = m_Depth;
    m_OldWidthSegments = m_WidthSegments;
    m_OldHeightSegments = m_HeightSegments;
    m_OldDepthSegments = m_DepthSegments;
}

void FCube::Backtrace() {
    m_Width = m_OldWidth;
    m_Height = m_OldHeight;
    m_Depth = m_OldDepth;
    m_WidthSegments = m_OldWidthSegments;
    m_HeightSegments = m_OldHeightSegments;
    m_DepthSegments = m_OldDepthSegments;
}

void FCube::BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width,
                       float height, float depth, uint32 gridX, uint32 gridY, FMeshBuffer* buffer) {
    const float segmentWidth = width / gridX;
    const float segmentHeight = height / gridY;

    const float widthHalf = width / 2;
    const float heightHalf = height / 2;
    const float depthHalf = depth / 2;

    const uint32 gridX1 = gridX + 1;
    const uint32 gridY1 = gridY + 1;

    int vertexCounter = 0;

    FVector3 vector;

    // Vertices
    for (int iy = 0; iy < gridY1; iy++) {
        const float y = iy * segmentHeight - heightHalf;

        for (int ix = 0; ix < gridX1; ix++) {

            const float x = ix * segmentWidth - widthHalf;

            FVertex vertice;

            SetAxisValue(vertice.Position, u, x * uDir);
            SetAxisValue(vertice.Position, v, y * vDir);
            SetAxisValue(vertice.Position, w, depthHalf);
            vertice.Position = (m_LocalTransform * FVector4(vertice.Position, 1.0f)).ToVector3();

            SetAxisValue(vertice.Normal, u, 0);
            SetAxisValue(vertice.Normal, v, 0);
            SetAxisValue(vertice.Normal, w, depth > 0 ? 1 : -1);

            vertice.TexCoord.x = ix / gridX;
            vertice.TexCoord.y = 1 - (iy / gridY);

            buffer->Vertexs.push_back(vertice);
            vertexCounter += 1;
        }
    }

    // Indices
    // 1. you need three indices to draw a single face
    // 2. a single segment consists of two faces
    // 3. so we need to generate six (2*3) indices per segment
    for (uint32 iy = 0; iy < gridY; iy++) {
        for (uint32 ix = 0; ix < gridX; ix++) {
            const uint32 a = m_NumberOfVertices + ix + gridX1 * iy;
            const uint32 b = m_NumberOfVertices + ix + gridX1 * (iy + 1);
            const uint32 c = m_NumberOfVertices + (ix + 1) + gridX1 * (iy + 1);
            const uint32 d = m_NumberOfVertices + (ix + 1) + gridX1 * iy;

            buffer->Indices.insert(buffer->Indices.end(), { a, b, d });
            buffer->Indices.insert(buffer->Indices.end(), { b, c, d });

            buffer->IndexCount += 6;
        }
    }

    m_NumberOfVertices += vertexCounter;
}