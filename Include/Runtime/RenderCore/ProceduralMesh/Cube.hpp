#pragma once

#include <Core/TypeRegistry/TypeRegistry.hpp>
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/ProceduralMesh/CubeParams.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

class Cube : public ProceduralMesh {
public:
    explicit Cube(const float width = 1.0f, const float height = 1.0f, const float depth = 1.0f,
                  const uint32_t widthSegments = 1, const uint32_t heightSegments = 1,
                  const uint32_t depthSegments = 1);
    explicit Cube(const CubeParams& params);
    ~Cube() override = default;

    virtual const std::string GetName() const override { return m_Params.GetTypeName(); }

    virtual MeshBuffer* GenerateBuffer() override;

    virtual void SetParams(const Params& params) override {
        if (params.GetTypeName() != "Cube") return;

        auto* cloned = static_cast<CubeParams*>(params.Clone());
        if (!cloned) return;

        m_Params.~CubeParams();
        new (&m_Params) CubeParams(std::move(*cloned));

        delete cloned;
    }

    // Getters
    const CubeParams& GetParams() const { return m_Params; }
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
    CubeParams m_Params;
};

} // namespace CZ
