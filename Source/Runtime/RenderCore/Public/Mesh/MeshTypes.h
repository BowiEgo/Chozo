#pragma once

#include "CoreMinimal.h"

#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct FVertex {
    FVector3 Position;
    FVector3 Normal;
    FVector2 UV;
    FVector3 Tangent   = FVector3::Zero;
    FVector3 Bitangent = FVector3::Zero;

    FVertex() = default;
    FVertex(const FVector3& pos, const FVector3& norm, const FVector2& uv,
            const FVector3& tangent = FVector3::Zero, const FVector3& bitangent = FVector3::Zero)
        : Position(pos), Normal(norm), UV(uv), Tangent(tangent), Bitangent(bitangent) {}
};

struct FIndex {
    uint32 V1, V2, V3;
};

// ===== Mesh Buffer (GPU ready) =====
struct FMeshBuffer {
    std::vector<FVertex> Vertices;
    std::vector<FIndex> Indices;
    FVector3 MinBounds;
    FVector3 MaxBounds;

    bool IsValid() const { return !Vertices.empty() && !Indices.empty(); }

    size_t GetVertexCount() const { return Vertices.size(); }
    size_t GetIndexCount() const { return Indices.size() * 3; }

    size_t GetVertexBufferSize() const { return Vertices.size() * sizeof(FVertex); }
    size_t GetIndexBufferSize() const { return Indices.size() * sizeof(FIndex); }

    FVector3 GetCenter() const { return (MinBounds + MaxBounds) * 0.5f; }
    FVector3 GetSize() const { return MaxBounds - MinBounds; }

    void UpdateBounds() {
        MinBounds = FVector3(FLT_MAX, FLT_MAX, FLT_MAX);
        MaxBounds = FVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

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
