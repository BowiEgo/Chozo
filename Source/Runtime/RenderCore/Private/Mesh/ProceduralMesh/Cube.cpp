#include "Cube.h"

SIMPLE_MESH_PROPERTY(FCubeParams, Width, float, Width)
SIMPLE_MESH_PROPERTY(FCubeParams, Height, float, Height)
SIMPLE_MESH_PROPERTY(FCubeParams, Depth, float, Depth)
SIMPLE_MESH_PROPERTY(FCubeParams, WidthSegments, uint32_t, WidthSegments)
SIMPLE_MESH_PROPERTY(FCubeParams, HeightSegments, uint32_t, HeightSegments)
SIMPLE_MESH_PROPERTY(FCubeParams, DepthSegments, uint32_t, DepthSegments)

static void SetAxisValue(FVector3& target, const std::string& axis, float value) {
    if (axis == "x") {
        target.x = value;
    } else if (axis == "y") {
        target.y = value;
    } else if (axis == "z") {
        target.z = value;
    }
}

FCube::FCube(const float width, const float height, const float depth, const uint32 widthSegments,
             const uint32 heightSegments, const uint32 depthSegments) {
    SetWidth(width);
    SetHeight(height);
    SetDepth(depth);
    SetWidthSegments(widthSegments);
    SetHeightSegments(heightSegments);
    SetDepthSegments(depthSegments);

    GenerateBuffer();
}

FCube::FCube(const FCubeParams& params) : m_Params(params) { GenerateBuffer(); }

// FCube::FCube(const FCubeParams& params, const FMeshBuffer* data)
//     : FProceduralMesh(data), m_Params(params) {}

FMeshBuffer* FCube::GenerateBuffer() {
    int totalVertexCounter = 0;

    m_Buffer.Clear();

    // build each side of the box geometry
    BuildPlane("z", "y", "x", -1, -1, m_Params.Depth, m_Params.Height, m_Params.Width,
               m_Params.DepthSegments, m_Params.HeightSegments, totalVertexCounter); // px
    BuildPlane("z", "y", "x", 1, -1, m_Params.Depth, m_Params.Height, -m_Params.Width,
               m_Params.DepthSegments, m_Params.HeightSegments, totalVertexCounter); // nx
    BuildPlane("x", "z", "y", 1, 1, m_Params.Width, m_Params.Depth, m_Params.Height,
               m_Params.WidthSegments, m_Params.DepthSegments, totalVertexCounter); // py
    BuildPlane("x", "z", "y", 1, -1, m_Params.Width, m_Params.Depth, -m_Params.Height,
               m_Params.WidthSegments, m_Params.DepthSegments, totalVertexCounter); // ny
    BuildPlane("x", "y", "z", 1, -1, m_Params.Width, m_Params.Height, m_Params.Depth,
               m_Params.WidthSegments, m_Params.HeightSegments, totalVertexCounter); // pz
    BuildPlane("x", "y", "z", -1, -1, m_Params.Width, m_Params.Height, -m_Params.Depth,
               m_Params.WidthSegments, m_Params.HeightSegments, totalVertexCounter); // nz

    return &m_Buffer;
}

void FCube::BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width,
                       float height, float depth, uint32 gridX, uint32 gridY,
                       int& totalVertexCounter) {
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

            vertice.UV.x = ix / gridX;
            vertice.UV.y = 1 - (iy / gridY);

            m_Buffer.Vertices.push_back(vertice);
            vertexCounter += 1;
        }
    }

    // Indices
    // 1. you need three indices to draw a single face
    // 2. a single segment consists of two faces
    // 3. so we need to generate six (2*3) indices per segment
    for (uint32 iy = 0; iy < gridY; iy++) {
        for (uint32 ix = 0; ix < gridX; ix++) {
            const uint32 a = totalVertexCounter + ix + gridX1 * iy;
            const uint32 b = totalVertexCounter + ix + gridX1 * (iy + 1);
            const uint32 c = totalVertexCounter + (ix + 1) + gridX1 * (iy + 1);
            const uint32 d = totalVertexCounter + (ix + 1) + gridX1 * iy;

            m_Buffer.Indices.insert(m_Buffer.Indices.end(), { a, b, d });
            m_Buffer.Indices.insert(m_Buffer.Indices.end(), { b, c, d });
        }
    }

    totalVertexCounter += vertexCounter;
}