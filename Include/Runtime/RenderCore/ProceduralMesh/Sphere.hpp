#pragma once

#include "MathUtils.h"
#include "MeshReflection.h"
#include "ProceduralMesh.h"
#include "SphereParams.h"

class RENDER_CORE_API FSphere : public FProceduralMesh {
public:
    explicit FSphere(float radius = 1.0f, uint32_t widthSegments = 32, uint32_t heightSegments = 16,
                     float phiStart = 0, float phiLength = ChozoUtils::Math::PI * 2,
                     float thetaStart = 0, float thetaLength = ChozoUtils::Math::PI);
    explicit FSphere(const FSphereParams& params);
    ~FSphere() override = default;

    virtual const std::string GetName() const override { return m_Params.GetTypeName(); }

    virtual FMeshBuffer* GenerateBuffer() override;
    virtual void SetParams(const IParams& params) override {
        if (params.GetTypeName() != "Sphere") return;

        auto* cloned = static_cast<FSphereParams*>(params.Clone());
        if (!cloned) return;

        m_Params.~FSphereParams();
        new (&m_Params) FSphereParams(std::move(*cloned));

        delete cloned;
    }

    // Getters
    const FSphereParams& GetParams() const { return m_Params; }
    float GetRadius() const { return m_Params.Radius; }
    uint32_t GetWidthSegments() const { return m_Params.WidthSegments; }
    uint32_t GetHeightSegments() const { return m_Params.HeightSegments; }
    float GetPhiStart() const { return m_Params.PhiStart; }
    float GetPhiLength() const { return m_Params.PhiLength; }
    float GetThetaStart() const { return m_Params.ThetaStart; }
    float GetThetaLength() const { return m_Params.ThetaLength; }

    // Setters
    void SetRadius(float radius) { m_Params.Radius = radius; }
    void SetWidthSegments(uint32_t widthSegments) { m_Params.WidthSegments = widthSegments; }
    void SetHeightSegments(uint32_t heightSegments) { m_Params.HeightSegments = heightSegments; }
    void SetPhiStart(float phiStart) { m_Params.PhiStart = phiStart; }
    void SetPhiLength(float phiLength) { m_Params.PhiLength = phiLength; }
    void SetThetaStart(float thetaStart) { m_Params.ThetaStart = thetaStart; }
    void SetThetaLength(float thetaLength) { m_Params.ThetaLength = thetaLength; }

private:
    void BuildPlane(int uDir, int vDir, float width, float height, float depth, uint32_t gridX,
                    uint32_t gridY);

private:
    FSphereParams m_Params;
};
