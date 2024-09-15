#pragma once

#include "Geometry.h"

namespace Chozo
{

    class QuadGeometry : public Geometry
    {
    public:
        QuadGeometry(const float& width = 1.0f, const float& height = 1.0f, const uint32_t& widthSegments = 1, const uint32_t& heightSegments = 1);
        ~QuadGeometry() = default;

        inline float GetWidth() { return m_Width; }
        inline void SetWidth(float& width) {
            Backup();
            m_Width = width; CallGenerate();
        }

        inline float GetHeight() { return m_Height; }
        inline void SetHeight(float& height) {
            Backup();
            m_Height = height; CallGenerate();
        }

        inline uint32_t GetWidthSegments() { return m_WidthSegments; }
        inline void SetWidthSegments(uint32_t& widthSegments) {
            Backup();
            m_WidthSegments = widthSegments; CallGenerate();
        }

        inline uint32_t GetHeightSegments() { return m_HeightSegments; }
        inline void SetHeightSegments(uint32_t& heightSegments) {
            Backup();
            m_HeightSegments = heightSegments; CallGenerate();
        }
    protected:
        virtual void Backup() override;
        virtual void Backtrace() override;
        virtual MeshBuffer* Generate() override;
    private:
        float m_Width, m_Height, m_Depth;
        uint32_t m_WidthSegments, m_HeightSegments;
        uint32_t m_NumberOfVertices = 0;

        float m_OldWidth, m_OldHeight;
        uint32_t m_OldWidthSegments, m_OldHeightSegments;
    };
}
