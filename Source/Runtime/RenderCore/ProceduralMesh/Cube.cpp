#include <Runtime/RenderCore/ProceduralMesh/Cube.hpp>

namespace CZ {

static void SetAxisValue(Vector3& target, const std::string& axis, float value) {
    if (axis == "x") {
        target.x = value;
    } else if (axis == "y") {
        target.y = value;
    } else if (axis == "z") {
        target.z = value;
    }
}

// Cube::Cube(const CubeParams& params, const MeshBuffer* data)
//     : FProceduralMesh(data), m_Params(params) {}

MeshBuffer* Cube::GenerateBuffer() {
    auto params = m_Params.As<CubeParamsObj>();

    (*this)->MeshBuffer.Clear();

    int totalVertexCounter = 0;

    // build each side of the box geometry
    BuildPlane("z", "y", "x", -1, -1, params->Depth, params->Height, params->Width,
               params->DepthSegments, params->HeightSegments, totalVertexCounter); // px
    BuildPlane("z", "y", "x", 1, -1, params->Depth, params->Height, -params->Width,
               params->DepthSegments, params->HeightSegments, totalVertexCounter); // nx
    BuildPlane("x", "z", "y", 1, 1, params->Width, params->Depth, params->Height,
               params->WidthSegments, params->DepthSegments, totalVertexCounter); // py
    BuildPlane("x", "z", "y", 1, -1, params->Width, params->Depth, -params->Height,
               params->WidthSegments, params->DepthSegments, totalVertexCounter); // ny
    BuildPlane("x", "y", "z", 1, -1, params->Width, params->Height, params->Depth,
               params->WidthSegments, params->HeightSegments, totalVertexCounter); // pz
    BuildPlane("x", "y", "z", -1, -1, params->Width, params->Height, -params->Depth,
               params->WidthSegments, params->HeightSegments, totalVertexCounter); // nz

    return &(*this)->MeshBuffer;
}

void Cube::BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width,
                      float height, float depth, uint32 gridX, uint32 gridY,
                      int& totalVertexCounter) {
    const float segmentWidth  = width / gridX;
    const float segmentHeight = height / gridY;

    const float widthHalf  = width / 2;
    const float heightHalf = height / 2;
    const float depthHalf  = depth / 2;

    const uint32 gridX1 = gridX + 1;
    const uint32 gridY1 = gridY + 1;

    int vertexCounter = 0;

    // Vertices
    for (uint32 iy = 0; iy < gridY1; iy++) {
        const float y = iy * segmentHeight - heightHalf;

        for (uint32 ix = 0; ix < gridX1; ix++) {

            const float x = ix * segmentWidth - widthHalf;

            Vertex vertice;

            SetAxisValue(vertice.Position, u, x * uDir);
            SetAxisValue(vertice.Position, v, y * vDir);
            SetAxisValue(vertice.Position, w, depthHalf);
            vertice.Position =
                (m_Obj->LocalTransform * Vector4(vertice.Position, 1.0f)).ToVector3();

            SetAxisValue(vertice.Normal, u, 0);
            SetAxisValue(vertice.Normal, v, 0);
            SetAxisValue(vertice.Normal, w, depth > 0 ? 1 : -1);

            vertice.UV.x = float(ix) / gridX;
            vertice.UV.y = 1 - (float(iy) / gridY);

            (*this)->MeshBuffer.Vertices.push_back(vertice);
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

            (*this)->MeshBuffer.Indices.insert((*this)->MeshBuffer.Indices.end(), { a, b, d });
            (*this)->MeshBuffer.Indices.insert((*this)->MeshBuffer.Indices.end(), { b, c, d });
        }
    }

    totalVertexCounter += vertexCounter;
}

} // namespace CZ
