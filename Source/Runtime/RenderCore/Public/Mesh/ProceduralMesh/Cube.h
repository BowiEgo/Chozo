#pragma once

#include "CubeParams.h"
#include "MeshReflection.h"
#include "ProceduralMesh.h"

class RENDER_CORE_API FCube : public FProceduralMesh {
public:
    explicit FCube(const float width = 1.0f, const float height = 1.0f, const float depth = 1.0f,
                   const uint32_t widthSegments = 1, const uint32_t heightSegments = 1,
                   const uint32_t depthSegments = 1);
    explicit FCube(const FCubeParams& params);
    ~FCube() override = default;

    virtual FMeshBuffer* GenerateBuffer() override;
    virtual void SetParams(const IParams& params) override {
        if (params.GetTypeName() != "Cube") return;

        auto* cloned = static_cast<FCubeParams*>(params.Clone());
        if (!cloned) return;

        m_Params.~FCubeParams();
        new (&m_Params) FCubeParams(std::move(*cloned));

        delete cloned;
    }

    // Getters
    const FCubeParams& GetParams() const { return m_Params; }
    float GetWidth() const { return m_Params.Width; }
    float GetHeight() const { return m_Params.Height; }
    float GetDepth() const { return m_Params.Depth; }
    uint32_t GetWidthSegments() const { return m_Params.WidthSegments; }
    uint32_t GetHeightSegments() const { return m_Params.HeightSegments; }
    uint32_t GetDepthSegments() const { return m_Params.DepthSegments; }

    // Setters
    void SetWidth(float width) { m_Params.Width = width; }
    void SetHeight(float height) { m_Params.Height = height; }
    void SetDepth(float depth) { m_Params.Depth = depth; }
    void SetWidthSegments(uint32_t widthSegments) { m_Params.WidthSegments = widthSegments; }
    void SetHeightSegments(uint32_t heightSegments) { m_Params.HeightSegments = heightSegments; }
    void SetDepthSegments(uint32_t depthSegments) { m_Params.DepthSegments = depthSegments; }

private:
    void BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width,
                    float height, float depth, uint32 gridX, uint32 gridY, int& totalVertexCounter);

private:
    FCubeParams m_Params;
};
