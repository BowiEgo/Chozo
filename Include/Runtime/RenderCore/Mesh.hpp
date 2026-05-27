#pragma once

#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/GraphicsBuffer.hpp>
#include <Runtime/RenderCore/Asset.hpp>

#include <cfloat>
#include <cstdint>
#include <vector>

namespace CZ {

struct Vertex {
    Vector3 Position;
    Vector3 Normal;
    Vector2 UV;
    Vector3 Tangent   = Vector3::Zero;
    Vector3 Bitangent = Vector3::Zero;

    Vertex() = default;
    Vertex(const Vector3& pos, const Vector3& norm, const Vector2& uv,
           const Vector3& tangent = Vector3::Zero, const Vector3& bitangent = Vector3::Zero)
        : Position(pos), Normal(norm), UV(uv), Tangent(tangent), Bitangent(bitangent) {}
};

struct Index {
    uint32_t V1, V2, V3;
};

// ===== Mesh Buffer (GPU ready) =====
struct MeshBuffer {
    std::vector<Vertex> Vertices;
    std::vector<Index> Indices;
    Vector3 MinBounds;
    Vector3 MaxBounds;

    bool IsValid() const { return !Vertices.empty() && !Indices.empty(); }

    size_t GetVertexCount() const { return Vertices.size(); }
    size_t GetIndexCount() const { return Indices.size() * 3; }

    size_t GetVertexBufferSize() const { return Vertices.size() * sizeof(Vertex); }
    size_t GetIndexBufferSize() const { return Indices.size() * sizeof(Index); }

    Vector3 GetCenter() const { return (MinBounds + MaxBounds) * 0.5f; }
    Vector3 GetSize() const { return MaxBounds - MinBounds; }

    void UpdateBounds() {
        MinBounds = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
        MaxBounds = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (const auto& vertex : Vertices) {
            MinBounds.x = std::min(MinBounds.x, vertex.Position.x);
            MinBounds.y = std::min(MinBounds.y, vertex.Position.y);
            MinBounds.z = std::min(MinBounds.z, vertex.Position.z);
            MaxBounds.x = std::max(MaxBounds.x, vertex.Position.x);
            MaxBounds.y = std::max(MaxBounds.y, vertex.Position.y);
            MaxBounds.z = std::max(MaxBounds.z, vertex.Position.z);
        }
    }

    void Optimize() {
        // DeduplicateVertices();
        // OptimizeIndexOrder();
        // CalculateNormalsIfMissing();

        // IndexCount = static_cast<uint32_t>(Indices.size());
    }

    void Clear() {
        Vertices.clear();
        Indices.clear();

        // Optional: release memory
        Vertices.shrink_to_fit();
        Indices.shrink_to_fit();
    }
};

struct MeshObj {
    std::string Name;
    MemoryType MemoryType = MemoryType::Unknown;

    GraphicsBuffer VertexBuffer;
    GraphicsBuffer IndexBuffer;

    MeshBuffer MeshBuffer;
    Matrix4 LocalTransform;

    std::string GetName() const { return Name; }
    GraphicsBuffer GetVertexBuffer() const { return VertexBuffer; }
    GraphicsBuffer GetIndexBuffer() const { return IndexBuffer; }
    uint32 GetIndexCount() const { return MeshBuffer.GetIndexCount(); }

    void Upload();
    void Draw(CommandList cmdList) const;
};

class Mesh : public Asset<MeshObj> {
public:
    Mesh() = default;
    explicit Mesh(MeshObj* obj) : Asset<MeshObj>(obj) {}

    AssetType GetType() const override { return AssetType::Mesh; }
    const std::string GetName() const override;
};

template <> struct AssetTraits<MeshObj> {
    using AssetClass = Mesh;
};

} // namespace CZ