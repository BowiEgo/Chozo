#include "Quad.h"

FQuad::FQuad() { GenerateBuffer(); }

FMeshBuffer* FQuad::GenerateBuffer() {
    m_Buffer.Clear();

    // std::vector<FVertex> vertices = {
    //     { FVector3(-1.0f, -1.0f, 0.0f), FVector3(0, 0, 1), FVector2(0.0f, 1.0f) },
    //     { FVector3(1.0f, -1.0f, 0.0f), FVector3(0, 0, 1), FVector2(1.0f, 1.0f) },
    //     { FVector3(1.0f, 1.0f, 0.0f), FVector3(0, 0, 1), FVector2(1.0f, 0.0f) },
    //     { FVector3(-1.0f, 1.0f, 0.0f), FVector3(0, 0, 1), FVector2(0.0f, 0.0f) }
    // };

    m_Buffer.Vertices = {
        { { -1.0f, -1.0f, 0.0f }, { 0, 0, 1 }, { 0.0f, 1.0f }, {}, {} }, // 左下
        { { 1.0f, -1.0f, 0.0f }, { 0, 0, 1 }, { 1.0f, 1.0f }, {}, {} },  // 右下
        { { 1.0f, 1.0f, 0.0f }, { 0, 0, 1 }, { 1.0f, 0.0f }, {}, {} },   // 右上
        { { -1.0f, 1.0f, 0.0f }, { 0, 0, 1 }, { 0.0f, 0.0f }, {}, {} }   // 左上
    };
    m_Buffer.Indices = { { 0, 1, 2 }, { 2, 3, 0 } };

    return &m_Buffer;
}