#pragma once

#include "Geometry.h"
#include "Chozo/Math/Math.h"

namespace Chozo
{

    class SphereGeometry : public Geometry
    {
    public:
        SphereGeometry(float radius = 1.0f, uint32_t widthSegments = 32, uint32_t heightSegments = 16,
            float phiStart = 0, float phiLength = Math::PI * 2, float thetaStart = 0, float thetaLength = Math::PI);
        ~SphereGeometry() = default;

        inline float GetRadius() { return m_Radius; }
        inline void SetRadius(float& radius) {
            Backup();
            m_Radius = radius;
            CallGenerate();
        }

        inline uint32_t GetWidthSegments() { return m_WidthSegments; }
        inline void SetWidthSegments(uint32_t& widthSegments) {
            Backup();
            m_WidthSegments = widthSegments;
            CallGenerate();
        }

        inline uint32_t GetHeightSegments() { return m_HeightSegments; }
        inline void SetHeightSegments(uint32_t& heightSegments) {
            Backup();
            m_HeightSegments = heightSegments;
            CallGenerate();
        }

        inline float GetPhiStart() { return m_PhiStart; }
        inline void SetPhiStart(float& phiStart) {
            Backup();
            m_PhiStart = phiStart;
            CallGenerate();
        }

        inline float GetPhiLength() { return m_PhiLength; }
        inline void SetPhiLength(float& phiLength) {
            Backup();
            m_PhiLength = phiLength;
            CallGenerate();
        }

        inline float GetThetaStart() { return m_ThetaStart; }
        inline void SetThetaStart(float& thetaStart) {
            Backup();
            m_ThetaStart = thetaStart;
            CallGenerate();
        }

        inline float GetThetaLength() { return m_ThetaLength; }
        inline void SetThetaLength(float& thetaLength) {
            Backup();
            m_ThetaLength = thetaLength;
            CallGenerate();
        }
    protected:
        virtual void Backup() override;
        virtual void Backtrace() override;
        virtual MeshBuffer* Generate() override;
    private:
        void BuildPlane(int uDir, int vDir, float width, float height, float depth, uint32_t gridX, uint32_t gridY);
    private:
        float m_Radius, m_PhiStart, m_PhiLength, m_ThetaStart, m_ThetaLength;
        uint32_t m_WidthSegments, m_HeightSegments;

        float m_OldRadius, m_OldPhiStart, m_OldPhiLength, m_OldThetaStart, m_OldThetaLength;
        uint32_t m_OldWidthSegments, m_OldHeightSegments;
    };
}
