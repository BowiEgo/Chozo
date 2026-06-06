#pragma once

#include "Runtime/RenderCore/MeshParams.hpp"
#include <Core/TypeRegistry/TypeRegistry.hpp>
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/ProceduralMesh/CubeParamsObj.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

class Cube : public ProceduralMesh {
public:
    using ProceduralMesh::ProceduralMesh;

    static Cube Create(const float width = 1.0f, const float height = 1.0f,
                       const float depth = 1.0f, const uint32_t widthSegments = 1,
                       const uint32_t heightSegments = 1, const uint32_t depthSegments = 1) {
        auto obj           = CZ_NEW(MEMORY_USAGE_RENDER, MeshObj);
        auto cubeParamsObj = CZ_NEW(MEMORY_USAGE_RENDER, CubeParamsObj, width, height, depth,
                                    widthSegments, heightSegments, depthSegments);
        auto result        = Cube(obj);
        result.SetParams(MeshParams(cubeParamsObj));
        Delete(cubeParamsObj);
        result.GenerateBuffer();
        return result;
    }

    static Cube Create(const MeshParams params) {
        auto obj    = CZ_NEW(MEMORY_USAGE_RENDER, MeshObj);
        auto result = Cube(obj);
        result.SetParams(params);
        result.GenerateBuffer();
        return result;
    }

    virtual const std::string GetName() const override { return m_Params->GetTypeName(); }

    virtual MeshBuffer* GenerateBuffer() override;

    virtual void SetParams(MeshParams params) override {
        if (params->GetTypeName() != "Cube") return;
        m_Params = params.Clone();
    }

    // Getters
    const MeshParams GetParams() const { return m_Params; }
    float GetWidth() { return m_Params.As<CubeParamsObj>()->Width; }
    float GetHeight() { return m_Params.As<CubeParamsObj>()->Height; }
    float GetDepth() { return m_Params.As<CubeParamsObj>()->Depth; }
    uint32_t GetWidthSegments() { return m_Params.As<CubeParamsObj>()->WidthSegments; }
    uint32_t GetHeightSegments() { return m_Params.As<CubeParamsObj>()->HeightSegments; }
    uint32_t GetDepthSegments() { return m_Params.As<CubeParamsObj>()->DepthSegments; }

    // Setters
    void SetWidth(float width) { m_Params.As<CubeParamsObj>()->Width = width; }
    void SetHeight(float height) { m_Params.As<CubeParamsObj>()->Height = height; }
    void SetDepth(float depth) { m_Params.As<CubeParamsObj>()->Depth = depth; }
    void SetWidthSegments(uint32_t widthSegments) {
        m_Params.As<CubeParamsObj>()->WidthSegments = widthSegments;
    }
    void SetHeightSegments(uint32_t heightSegments) {
        m_Params.As<CubeParamsObj>()->HeightSegments = heightSegments;
    }
    void SetDepthSegments(uint32_t depthSegments) {
        m_Params.As<CubeParamsObj>()->DepthSegments = depthSegments;
    }

private:
    void BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width,
                    float height, float depth, uint32 gridX, uint32 gridY, int& totalVertexCounter);
};

} // namespace CZ
