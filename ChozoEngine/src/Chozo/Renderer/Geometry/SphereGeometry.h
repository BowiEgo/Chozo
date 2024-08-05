#pragma once

#include "Geometry.h"
#include "Chozo/Math/Math.h"

namespace Chozo
{

    class SphereGeometry : public Geometry
    {
    public:
        SphereGeometry(float radius = 1.0f, unsigned int widthSegments = 32, unsigned int heightSegments = 16, float phiStart = 0, float phiLength = Math::PI * 2, float thetaStart = 0, float thetaLength = Math::PI);
        ~SphereGeometry() = default;
    private:
        void BuildPlane(int uDir, int vDir, float width, float height, float depth, uint32_t gridX, uint32_t gridY);
    private:
        float m_Width, m_Height, m_Depth;
        uint32_t m_WidthSegments, m_HeightSegments, m_DepthSegments;
        uint32_t m_NumberOfVertices = 0;
    };
}
