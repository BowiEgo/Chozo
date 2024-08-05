#pragma once

#include "Geometry.h"

namespace Chozo
{

    class BoxGeometry : public Geometry
    {
    public:
        BoxGeometry(const float& width = 1.0f, const float& height = 1.0f, const float& depth = 1.0f, const uint32_t& widthSegments = 1, const uint32_t& heightSegments = 1, const uint32_t& depthSegments = 1);
        ~BoxGeometry() = default;
    private:
        void BuildPlane(std::string u, std::string v, std::string w, int uDir, int vDir, float width, float height, float depth, uint32_t gridX, uint32_t gridY);
    private:
        float m_Width, m_Height, m_Depth;
        uint32_t m_WidthSegments, m_HeightSegments, m_DepthSegments;
        uint32_t m_NumberOfVertices = 0;
    };
}
