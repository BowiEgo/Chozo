#pragma once

#include "MathUtils.h"
#include "ProceduralMesh.h"

class RENDER_CORE_API FSphere : public FProceduralMesh {
public:
    explicit FSphere(float radius = 1.0f, uint32 widthSegments = 32, uint32 heightSegments = 16,
                     float phiStart = 0, float phiLength = ChozoUtils::Math::PI * 2,
                     float thetaStart = 0, float thetaLength = ChozoUtils::Math::PI);
    ~FSphere() override = default;

    inline float GetRadius() const { return m_Radius; }
    inline void SetRadius(const float& radius) {
        Backup();
        m_Radius = radius;
    }

    inline uint32 GetWidthSegments() const { return m_WidthSegments; }
    inline void SetWidthSegments(const uint32& widthSegments) {
        Backup();
        m_WidthSegments = widthSegments;
    }

    inline uint32 GetHeightSegments() const { return m_HeightSegments; }
    inline void SetHeightSegments(const uint32& heightSegments) {
        Backup();
        m_HeightSegments = heightSegments;
    }

    inline float GetPhiStart() const { return m_PhiStart; }
    inline void SetPhiStart(const float& phiStart) {
        Backup();
        m_PhiStart = phiStart;
    }

    inline float GetPhiLength() const { return m_PhiLength; }
    inline void SetPhiLength(const float& phiLength) {
        Backup();
        m_PhiLength = phiLength;
    }

    inline float GetThetaStart() const { return m_ThetaStart; }
    inline void SetThetaStart(const float& thetaStart) {
        Backup();
        m_ThetaStart = thetaStart;
    }

    inline float GetThetaLength() const { return m_ThetaLength; }
    inline void SetThetaLength(const float& thetaLength) {
        Backup();
        m_ThetaLength = thetaLength;
    }

protected:
    void Backup() override;
    void Backtrace() override;
    FMeshBuffer* Generate() override;

private:
    void BuildPlane(int uDir, int vDir, float width, float height, float depth, uint32 gridX,
                    uint32 gridY);

private:
    float m_Radius, m_PhiStart, m_PhiLength, m_ThetaStart, m_ThetaLength;
    uint32 m_WidthSegments, m_HeightSegments;

    float m_OldRadius, m_OldPhiStart, m_OldPhiLength, m_OldThetaStart, m_OldThetaLength;
    uint32 m_OldWidthSegments, m_OldHeightSegments;
};
