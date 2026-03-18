#pragma once

#include "CoreMinimal.h"
#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct FVertex {
    FVector3 Position;
    FVector3 Normal;
    FVector2 TexCoord;
    FVector3 Tangent;
    FVector3 Bitangent;
};

struct FIndex {
    uint32 V1, V2, V3;
};

struct FMeshBuffer {
    std::vector<FVertex> Vertexs;
    std::vector<FIndex> Indices;
    uint32 IndexCount = 0;
};